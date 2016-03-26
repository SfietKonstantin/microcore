import copy
from exception import CheckException


class Transformer:
    def __init__(self, in_data):
        # type: (str) -> Transformer
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
    def _make_setter_type(c_type):
        # type: (str) -> str
        if Transformer._is_simple_type(c_type):
            return c_type + " "
        else:
            return c_type + " &&"

    @staticmethod
    def _make_setter_impl(c_type, name):
        # type: (str, str) -> str
        if Transformer._is_simple_type(c_type):
            return name
        else:
            return "std::move(" + name + ")"

    def _check(self):
        # type: () -> None
        if "name" not in self.in_data:
            raise CheckException("\"name\" is a mandatory field")
        if "module" not in self.in_data:
            raise CheckException("\"module\" is a mandatory field")
        if "properties" not in self.in_data:
            raise CheckException("\"properties\" is a mandatory field")
        for bean_property in self.in_data["properties"]:
            if "name" not in bean_property:
                raise CheckException("For %s \"name\" is a mandatory property field" % repr(bean_property))
            if "type" not in bean_property:
                raise CheckException("For %s \"type\" is a mandatory property field" % repr(bean_property))
            if "access" not in bean_property:
                raise CheckException("For %s \"access\" is a mandatory property field" % repr(bean_property))
            if bean_property["access"] != "c" and bean_property["access"] != "r" and bean_property["access"] != "rw":
                raise CheckException("For %s \"access\" must be c, r or rw" % repr(bean_property))

    def _fill_property(self, bean_property):
        # type: (dict) -> dict
        return {"name": bean_property["name"], "access": bean_property["access"]}

    def _fill(self):
        # type: () -> None
        self.out_data["name"] = self.in_data["name"]
        self.out_data["module"] = self.in_data["module"]
        self.out_data["properties"] = []
        for bean_property in self.in_data["properties"]:
            out_property = self._fill_property(bean_property)
            self.out_data["properties"].append(out_property)

    def generate(self):
        # type: () -> None
        self._check()
        self._fill()


class BeanTransformer(Transformer, object):
    def __init__(self, in_data):
        # type: (str) -> BeanTransformer
        super(BeanTransformer, self).__init__(in_data)
        self.list_type = "std::vector"
        self.list_include = "vector"

    def _is_const(self):
        # type: () -> bool
        for bean_property in self.in_data["properties"]:
            if bean_property["access"] != "c":
                return False
        return True

    def _fill_property(self, bean_property):
        # type: (dict) -> dict
        returned = super(BeanTransformer, self)._fill_property(bean_property)

        if "list" in bean_property and bean_property["list"]:
            returned["type"] = "%s<%s>" % (self.list_type, bean_property["type"])
        else:
            returned["type"] = bean_property["type"]

        returned["getter"] = self._make_getter(returned["type"], bean_property["name"])
        returned["initial_value"] = self._make_initial_value(returned["type"])
        returned["setter_type"] = self._make_setter_type(returned["type"])
        returned["setter_impl"] = self._make_setter_impl(returned["type"], bean_property["name"])
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

    def _gen_includes(self):
        # type: () -> None
        includes = []
        for bean_property in self.in_data["properties"]:
            if BeanTransformer._is_qt_type(bean_property["type"]):
                if bean_property["type"] not in includes:
                    includes.append(bean_property["type"])
        includes.sort()
        if self._has_list():
            includes.insert(0, self.list_include)

        self.out_data["includes"] = includes

    def _fill(self):
        # type: () -> None
        super(BeanTransformer, self)._fill()
        self.out_data["const"] = self._is_const()
        self._gen_includes()

    def generate(self):
        # type: () -> None
        super(BeanTransformer, self).generate()


class QtBeanTransformer(BeanTransformer, object):
    def __init__(self, in_data):
        super(QtBeanTransformer, self).__init__(in_data)
        self.list_type = "QList"
        self.list_include = "QList"

