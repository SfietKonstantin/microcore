import os
from mako.template import Template


class Generator:
    def __init__(self, data, outdir, outfile):
        # type: (dict, str, str) -> Generator
        self.data = data
        self.outdir = outdir
        self.outfile = outfile
        self.data["outfile"] = outfile
        self.indent = True

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

    def _indent(self, text):
        # type: (str) -> str
        if self.indent:
            return "    " + ("\n    ".join(text.strip().split("\n")))
        else:
            return text

    def _recursive_fill_templates(self, data):
        # type: (dict) -> None
        for sub_class in data["classes"]:
            self._recursive_fill_templates(sub_class)
            sub_class["h_nested"] = self._indent(self.h_nested_template.render(**sub_class))
            sub_class["c_nested"] = self.c_nested_template.render(**sub_class)

    def _render(self):
        with open(os.path.join(self.outdir, self.outfile + ".h"), 'w') as h:
            h.write(self.h_template.render(**self.data))
            h.close()
        with open(os.path.join(self.outdir, self.outfile + ".cpp"), 'w') as cpp:
            cpp.write(self.c_template.render(**self.data))
            cpp.close()

    def generate(self):
        # type: () -> None
        self._create_templates()
        self._recursive_fill_templates(self.data)
        self._render()


class BeanGenerator(Generator, object):
    def __init__(self, data, outdir, outfile):
        super(BeanGenerator, self).__init__(data, outdir, outfile)
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
        super(QtBeanGenerator, self).__init__(data, outdir, parent_outfile + "object")
        self.indent = False
        self.data["parent_outfile"] = parent_outfile
        self.h_template_file = "beanobject.h.tpl"
        self.c_template_file = "beanobject.cpp.tpl"
        self.h_nested_template_file = "beanobject-nested.h.tpl"
        self.c_nested_template_file = "beanobject-nested.cpp.tpl"
