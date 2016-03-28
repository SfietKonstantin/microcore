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
    def _recursive_check(bean_property, names, classes):
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

        if bean_property["type"] == "class":
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
                Transformer._recursive_check(sub_bean_property, sub_names, sub_classes)

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
            Transformer._recursive_check(bean_property, names, classes)

    def _fill_property(self, bean_property, parent_classes):
        # type: (dict, list) -> dict
        return {"name": bean_property["name"], "access": bean_property["access"]}

    def _fill_properties(self, in_data, out_data, parent_classes):
        # type: (dict, dict, list) -> None
        out_data["properties"] = []
        out_data["classes"] = []
        for bean_property in in_data["properties"]:
            out_property = self._fill_property(bean_property, parent_classes)
            out_data["properties"].append(out_property)
        for bean_property in in_data["properties"]:
            if bean_property["type"] == "class":
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
        nested_type = bean_property["type"]
        if bean_property["type"] == "class":
            bean_type = bean_property["class_name"]
            nested_type = "::".join(parent_classes) + "::" + bean_type

        if "list" in bean_property and bean_property["list"]:
            returned["type_type"] = "list"
        elif self._is_simple_type(bean_type):
            returned["type_type"] = "simple"
        else:
            returned["type_type"] = "object"

        returned["type"] = bean_type
        returned["nested_type"] = nested_type
        returned["getter"] = self._make_getter(bean_type, bean_property["name"])
        returned["initial_value"] = self._make_initial_value(bean_type)
        if bean_property["access"] == "rw":
            returned["setter"] = self._make_setter(bean_property["name"])
        return returned

    @staticmethod
    def _is_qt_type(c_type):
        # type: (str) -> str
        return c_type.startswith("Q")

    def _has_list(self):
        # type: () -> bool
        for bean_property in self.in_data["properties"]:
            if "list" in bean_property and bean_property["list"]:
                return True
        return False

    @staticmethod
    def _recursive_gen_includes(bean_property, includes):
        # type: (dict, list) -> None
        if BeanTransformer._is_qt_type(bean_property["type"]):
            if bean_property["type"] not in includes:
                includes.append(bean_property["type"])

        if bean_property["type"] == "class":
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
        if bean_property["type"] == "class":
            returned["qt_class"] = "".join(parent_classes) + bean_property["class_name"] + "Object"
            returned["qt_type"] = returned["qt_class"] + " *"
            returned["is_qt_object"] = True
        else:
            returned["qt_type"] = bean_property["type"]
            returned["is_qt_object"] = False

        if "list" in bean_property and bean_property["list"]:
            self.out_data["has_list"] = True

        returned["nested_name"] = "::".join(parent_classes) + "::" + bean_property["name"]
        return returned
