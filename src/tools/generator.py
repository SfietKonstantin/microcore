import os
from mako.template import Template


class Generator:
    def __init__(self, data, outdir, outfile, suffix=""):
        # type: (dict, str, str, str) -> Generator
        self.data = data
        self.outdir = outdir
        self.outfile = outfile
        self.suffix = suffix
        self.data["outfile"] = outfile + suffix
        self.indent_h = True
        self.indent_c = False

        self.h_template = None
        self.c_template = None
        self.h_nested_template = None
        self.c_nested_template = None

    @staticmethod
    def _get_tpl(tpl):
        return os.path.join(os.path.dirname(os.path.realpath(__file__)), tpl)

    def _create_templates(self):
        # type: () -> None
        pass

    @staticmethod
    def _indent(text):
        # type: (str) -> str
        splitted = text.strip().split("\n")
        returned = ""
        for line in splitted:
            if len(line) > 0:
                returned += "    " + line + "\n"
            else:
                returned += "\n"
        return returned[0:-1]

    def _indent_h(self, text):
        # type: (str) -> str
        if self.indent_h:
            return Generator._indent(text)
        else:
            return text

    def _indent_c(self, text):
        # type: (str) -> str
        if self.indent_c:
            return Generator._indent(text)
        else:
            return text

    def _recursive_fill_templates(self, data):
        # type: (dict) -> None
        for sub_class in data["classes"]:
            self._recursive_fill_templates(sub_class)
            if self.h_nested_template is not None:
                sub_class["h_nested"] = self._indent_h(self.h_nested_template.render(**sub_class))
            if self.c_nested_template is not None:
                sub_class["c_nested"] = self._indent_c(self.c_nested_template.render(**sub_class))

    def _render(self):
        with open(os.path.join(self.outdir, self.outfile + self.suffix + ".h"), 'w') as h:
            h.write(self.h_template.render(**self.data))
            h.close()
        with open(os.path.join(self.outdir, self.outfile + self.suffix + ".cpp"), 'w') as cpp:
            cpp.write(self.c_template.render(**self.data))
            cpp.close()

    def generate(self):
        # type: () -> None
        self._create_templates()
        self._recursive_fill_templates(self.data)
        self._render()


class BeanGenerator(Generator, object):
    def __init__(self, data, outdir, outfile, suffix=""):
        # type: (dict, str, str) -> BeanGenerator
        super(BeanGenerator, self).__init__(data, outdir, outfile, suffix)
        self.h_template_file = "bean.h.tpl"
        self.c_template_file = "bean.cpp.tpl"
        self.h_nested_template_file = "bean-nested.h.tpl"
        self.c_nested_template_file = "bean-nested.cpp.tpl"

    def _create_templates(self):
        # type: () -> None
        self.h_template = Template(filename=Generator._get_tpl(self.h_template_file))
        self.c_template = Template(filename=Generator._get_tpl(self.c_template_file))
        self.h_nested_template = Template(filename=Generator._get_tpl(self.h_nested_template_file))
        self.c_nested_template = Template(filename=Generator._get_tpl(self.c_nested_template_file))


class QtBeanGenerator(BeanGenerator, object):
    def __init__(self, data, outdir, parent_outfile):
        super(QtBeanGenerator, self).__init__(data, outdir, parent_outfile, "object")
        self.indent_h = False
        self.data["parent_outfile"] = parent_outfile
        self.h_template_file = "beanobject.h.tpl"
        self.c_template_file = "beanobject.cpp.tpl"
        self.h_nested_template_file = "beanobject-nested.h.tpl"
        self.c_nested_template_file = "beanobject-nested.cpp.tpl"


class FactoryGenerator(Generator, object):
    def __init__(self, data, outdir, parent_outfile):
        # type: (dict, str, str) -> FactoryGenerator
        super(FactoryGenerator, self).__init__(data, outdir, parent_outfile, "requestfactory")
        self.h_types_template = None
        self.data["parent_outfile"] = parent_outfile

    def _render(self):
        super(FactoryGenerator, self)._render()
        with open(os.path.join(self.outdir, self.outfile + "types.h"), 'w') as types:
            types.write(self.h_types_template.render(**self.data))
            types.close()


class JsonFactoryGenerator(FactoryGenerator, object):
    def __init__(self, data, outdir, parent_outfile):
        super(JsonFactoryGenerator, self).__init__(data, outdir, parent_outfile)
        self.indent_c = True

    def _create_templates(self):
        # type: () -> None
        self.h_template = Template(filename=Generator._get_tpl("factory.h.tpl"))
        self.c_template = Template(filename=Generator._get_tpl("factoryjson.cpp.tpl"))
        # self.h_nested_template = Template(filename=Generator._get_tpl("factoryjson-nested.h.tpl"))
        self.c_nested_template = Template(filename=Generator._get_tpl("factoryjson-nested.cpp.tpl"))
        self.h_types_template = Template(filename=Generator._get_tpl("typesjson.h.tpl"))
