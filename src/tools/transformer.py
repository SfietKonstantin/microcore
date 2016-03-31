from exception import CheckException


class Transformer:
    def __init__(self, in_data):
        # type: (dict) -> Transformer
        self.in_data = in_data
        self.out_data = {}

    @staticmethod
    def _upper(name):
        # type: (str) -> str
        return name[0].upper() + name[1:]

    @staticmethod
    def _make_getter(c_type, name):
        # type: (str, str) -> str
        if c_type == "bool":
            return "is" + Transformer._upper(name)
        else:
            return name

    @staticmethod
    def _make_setter(name):
        # type: (str) -> str
        return "set" + Transformer._upper(name)

    @staticmethod
    def _is_simple_type(c_type):
        # type: (str) -> bool
        return c_type == "int" or c_type == "double" or c_type == "bool"

    @staticmethod
    def _make_initial_value(c_type):
        # type: (str) -> str
        if c_type == "int":
            return "0"
        elif c_type == "double":
            return "0."
        elif c_type == "bool":
            return "false"
        else:
            return ""

    @staticmethod
    def _is_list(bean_property):
        # type: (dict) -> boolean
        return "list" in bean_property and bean_property["list"]

    @staticmethod
    def _is_class(bean_property):
        # type: (dict) -> boolean
        return bean_property["type"] == "class"

    def _recursive_check_property(self, bean_property, names, classes):
        # type: (dict, list, list) -> None
        if "name" not in bean_property:
            raise CheckException("For %s \"name\" is a mandatory property field" % repr(bean_property))
        if bean_property["name"] in names:
            raise CheckException("For %s, name %s is already defined" % (repr(bean_property),
                                                                         bean_property["name"]))
        names.append(bean_property["name"])
        if "type" not in bean_property:
            raise CheckException("For %s \"type\" is a mandatory property field" % repr(bean_property))
        if "access" not in bean_property:
            raise CheckException("For %s \"access\" is a mandatory property field" % repr(bean_property))
        if bean_property["access"] != "c" and bean_property["access"] != "r" and bean_property["access"] != "rw":
            raise CheckException("For %s \"access\" must be c, r or rw" % repr(bean_property))

        if Transformer._is_class(bean_property):
            if "class_name" not in bean_property:
                raise CheckException("For %s \"class_name\" is a mandatory property field" % repr(bean_property))
            if "properties" not in bean_property:
                raise CheckException("For %s \"properties\" is a mandatory property field" % repr(bean_property))
            if bean_property["class_name"] in classes:
                raise CheckException("For %s, type %s is already defined" % (repr(bean_property),
                                                                             bean_property["class_name"]))
            classes.append(bean_property["class_name"])
            sub_names = []
            sub_classes = []
            for sub_bean_property in bean_property["properties"]:
                self._recursive_check_property(sub_bean_property, sub_names, sub_classes)

    def _check(self):
        # type: () -> None
        if "name" not in self.in_data:
            raise CheckException("\"name\" is a mandatory field")
        if "module" not in self.in_data:
            raise CheckException("\"module\" is a mandatory field")
        if "properties" not in self.in_data:
            raise CheckException("\"properties\" is a mandatory field")

        names = []
        classes = []
        for bean_property in self.in_data["properties"]:
            self._recursive_check_property(bean_property, names, classes)

    @staticmethod
    def _recursive_has_list(in_data):
        for bean_property in in_data["properties"]:
            if Transformer._is_list(bean_property):
                return True
            if Transformer._is_class(bean_property):
                if Transformer._recursive_has_list(bean_property):
                    return True
        return False

    def _has_list(self):
        # type: () -> bool
        return Transformer._recursive_has_list(self.in_data)

    def _fill_property(self, bean_property, parent_classes):
        # type: (dict, list) -> dict

        bean_type = bean_property["type"]
        nested_type = bean_property["type"]
        if Transformer._is_class(bean_property):
            bean_type = bean_property["class_name"]
            nested_type = "::".join(parent_classes) + "::" + bean_type

        return {
            "name": bean_property["name"],
            "access": bean_property["access"],
            "type": bean_type,
            "nested_type": nested_type
        }

    def _fill_properties(self, in_data, out_data, parent_classes):
        # type: (dict, dict, list) -> None
        out_data["properties"] = []
        out_data["classes"] = []
        for bean_property in in_data["properties"]:
            out_property = self._fill_property(bean_property, parent_classes)
            out_data["properties"].append(out_property)
        for bean_property in in_data["properties"]:
            if Transformer._is_class(bean_property):
                out_class = self._fill_class(bean_property, parent_classes)
                out_data["classes"].append(out_class)

    def _fill_class(self, bean_property, parent_classes):
        # type: (dict, list) -> dict
        returned = {
            "module": self.in_data["module"],
            "name": bean_property["class_name"],
            "nested_name": "::".join(parent_classes) + "::" + bean_property["class_name"]
        }
        new_parent_classes = list(parent_classes)
        new_parent_classes.append(bean_property["class_name"])
        self._fill_properties(bean_property, returned, new_parent_classes)
        return returned

    def _fill(self):
        # type: () -> None
        self.out_data["name"] = self.in_data["name"]
        self.out_data["module"] = self.in_data["module"]
        self._fill_properties(self.in_data, self.out_data, [self.in_data["name"]])

    def generate(self):
        # type: () -> None
        self._check()
        self._fill()


class BeanTransformer(Transformer, object):
    def __init__(self, in_data):
        # type: (dict) -> BeanTransformer
        super(BeanTransformer, self).__init__(in_data)
        self.list_include = "vector"

    @staticmethod
    def _is_const(out_data):
        # type: (dict) -> bool
        for bean_property in out_data["properties"]:
            if bean_property["access"] != "c":
                return False
        return True

    def _fill_property(self, bean_property, parent_classes):
        # type: (dict, list) -> dict
        returned = super(BeanTransformer, self)._fill_property(bean_property, parent_classes)

        bean_type = bean_property["type"]

        if Transformer._is_list(bean_property):
            returned["type_type"] = "list"
        elif Transformer._is_simple_type(bean_type):
            returned["type_type"] = "simple"
        else:
            returned["type_type"] = "object"

        returned["getter"] = self._make_getter(bean_type, bean_property["name"])
        returned["initial_value"] = self._make_initial_value(bean_type)
        if bean_property["access"] == "rw":
            returned["setter"] = self._make_setter(bean_property["name"])
        return returned

    @staticmethod
    def _is_qt_type(c_type):
        # type: (str) -> str
        return c_type.startswith("Q")

    @staticmethod
    def _recursive_gen_includes(bean_property, includes):
        # type: (dict, list) -> None
        if BeanTransformer._is_qt_type(bean_property["type"]):
            if bean_property["type"] not in includes:
                includes.append(bean_property["type"])

        if Transformer._is_class(bean_property):
            for sub_bean_property in bean_property["properties"]:
                BeanTransformer._recursive_gen_includes(sub_bean_property, includes)

    def _gen_includes(self):
        # type: () -> None
        includes = []
        for bean_property in self.in_data["properties"]:
            self._recursive_gen_includes(bean_property, includes)
        includes.sort()
        if self._has_list():
            includes.insert(0, self.list_include)

        self.out_data["includes"] = includes

    @staticmethod
    def _recursive_is_const(out_data):
        out_data["const"] = BeanTransformer._is_const(out_data)
        for bean_class in out_data["classes"]:
            BeanTransformer._recursive_is_const(bean_class)

    def _fill(self):
        # type: () -> None
        super(BeanTransformer, self)._fill()
        BeanTransformer._recursive_is_const(self.out_data)
        self._gen_includes()

    def generate(self):
        # type: () -> None
        super(BeanTransformer, self).generate()


class QtBeanTransformer(BeanTransformer, object):
    def __init__(self, in_data):
        super(QtBeanTransformer, self).__init__(in_data)
        self.out_data["has_list"] = False

    def _fill_class(self, bean_property, parent_classes):
        # type: (dict, list) -> dict
        returned = super(QtBeanTransformer, self)._fill_class(bean_property, parent_classes)
        returned["name"] = "".join(parent_classes) + returned["name"]
        return returned

    def _fill_property(self, bean_property, parent_classes):
        # type: (dict, list) -> dict
        returned = super(QtBeanTransformer, self)._fill_property(bean_property, parent_classes)
        if Transformer._is_class(bean_property):
            returned["qt_class"] = "".join(parent_classes) + bean_property["class_name"] + "Object"
            returned["qt_type"] = returned["qt_class"] + " *"
            returned["is_qt_object"] = True
        else:
            returned["qt_type"] = bean_property["type"]
            returned["is_qt_object"] = False

        if Transformer._is_list(bean_property):
            self.out_data["has_list"] = True

        returned["nested_name"] = "::".join(parent_classes) + "::" + bean_property["name"]
        return returned


class JsonFactoryTransformer(Transformer, object):
    def __init__(self, in_data):
        super(JsonFactoryTransformer, self).__init__(in_data)

    @staticmethod
    def _conversion_method(c_type):
        # type: (str) -> str
        if c_type == "QString":
            return "toString"
        elif c_type == "int":
            return "toInt"
        elif c_type == "double":
            return "toDouble"
        elif c_type == "bool":
            return "toBool"
        elif c_type == "class":
            return "toObject"
        else:
            return ""

    def _check(self):
        # type: () -> None
        super(JsonFactoryTransformer, self)._check()
        if "source" not in self.in_data:
            raise CheckException("\"source\" is a mandatory field")
        if self.in_data["source"] != "json":
            raise CheckException("Only json \"source\" is currently supported")

    def _recursive_check_property(self, bean_property, names, classes):
        # type: (dict, list, list) -> None
        super(JsonFactoryTransformer, self)._recursive_check_property(bean_property, names, classes)
        if "json_path" not in bean_property:
            raise CheckException("For %s \"json_path\" is a mandatory property field" % repr(bean_property))
        if bean_property["json_path"].startswith("/"):
            raise CheckException("For %s \"json_path\" must not start with a /" % repr(bean_property))
        if self._conversion_method(bean_property["type"]) == "":
            raise CheckException("For %s, type \"%s\" cannot be handled by JSON" % (repr(bean_property),
                                                                                    bean_property["type"]))

    @staticmethod
    def _add_root(path):
        # type (str) -> str
        return "root/" + path

    @staticmethod
    def _get_splitted_json_path_prefix(path):
        # type (str) -> list
        splitted = path.split("/")
        if len(splitted) <= 1:
            return []
        return splitted[0:-1]

    @staticmethod
    def _get_json_path_suffix(path):
        # type (str) -> str
        return path.split("/")[-1]

    @staticmethod
    def _recursive_build_json_path_tree(node, root, leaves):
        # type: (dict, str, list) -> None
        if root not in node:
            node[root] = {}
        if len(leaves) > 0:
            JsonFactoryTransformer._recursive_build_json_path_tree(node[root], leaves[0], leaves[1:])

    @staticmethod
    def _add_to_json_path_tree(tree, path):
        # type (dict, list) -> None
        JsonFactoryTransformer._recursive_build_json_path_tree(tree, path[0], path[1:])

    @staticmethod
    def _recursive_build_json_path_flat_tree(flat_tree, hiearchy, node):
        # type: (list, list, dict) -> None
        for key in node:
            if len(hiearchy) > 0:
                flat_tree.append((hiearchy, key))
            new_hiearchy = list(hiearchy)
            new_hiearchy.append(key)
            JsonFactoryTransformer._recursive_build_json_path_flat_tree(flat_tree, new_hiearchy, node[key])

    @staticmethod
    def _convert_to_json_path_flat_tree(flat_tree, tree):
        JsonFactoryTransformer._recursive_build_json_path_flat_tree(flat_tree, [], tree)

    def _fill_properties(self, in_data, out_data, parent_classes):
        # type: (dict, dict, list) -> None
        super(JsonFactoryTransformer, self)._fill_properties(in_data, out_data, parent_classes)
        tree = {}
        for bean_property in in_data["properties"]:
            root_path = JsonFactoryTransformer._add_root(bean_property["json_path"])
            path = JsonFactoryTransformer._get_splitted_json_path_prefix(root_path)
            JsonFactoryTransformer._add_to_json_path_tree(tree, path)
        flat_tree = []
        JsonFactoryTransformer._convert_to_json_path_flat_tree(flat_tree, tree)
        out_data["json_tree"] = flat_tree

    def _fill_property(self, bean_property, parent_classes):
        # type: (dict, list) -> dict
        returned = super(JsonFactoryTransformer, self)._fill_property(bean_property, parent_classes)
        returned["json_optional"] = "json_optional" in bean_property and bean_property["json_optional"]

        if Transformer._is_list(bean_property):
            if Transformer._is_class(bean_property):
                returned["json_type"] = "objectarray"
            else:
                returned["json_type"] = "array"
        elif Transformer._is_class(bean_property):
            returned["json_type"] = "object"
        else:
            returned["json_type"] = "simple"

        json_path = bean_property["json_path"]
        root_path = JsonFactoryTransformer._add_root(bean_property["json_path"])
        returned["json_path"] = json_path
        returned["json_prefix"] = JsonFactoryTransformer._get_splitted_json_path_prefix(root_path)
        returned["json_suffix"] = JsonFactoryTransformer._get_json_path_suffix(root_path)
        returned["json_conversion_method"] = JsonFactoryTransformer._conversion_method(bean_property["type"])
        return returned

    def _fill_class(self, bean_property, parent_classes):
        # type: (dict, list) -> dict
        returned = super(JsonFactoryTransformer, self)._fill_class(bean_property, parent_classes)
        returned["root_name"] = self.in_data["name"]
        return returned

    def _gen_includes(self):
        # type: () -> None
        includes = []
        if self._has_list():
            includes.append("QJsonArray")

        self.out_data["includes"] = includes

    def _fill(self):
        # type: () -> None
        super(JsonFactoryTransformer, self)._fill()
        self._gen_includes()
