#!/usr/bin/env python
from mako.template import Template
import yaml
import argparse
import os
from exception import MicroGenException
from generator import BeanGenerator, QtBeanGenerator, JsonFactoryGenerator
from transformer import BeanTransformer, QtBeanTransformer, JsonFactoryTransformer


def _get_tpl(tpl):
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), tpl)


class JsonFactoryGeneratorOld:
    def __init__(self, data, input, outdir):
        self.data = data
        self.input = input
        self.outdir = outdir
                
    def render(self):
        self._check()
        self._build_json_object_tree()
        self._enrich_data()
        self._enrich_properties()
        
        outputFile = os.path.splitext(os.path.basename(self.input))[0]
        with open(os.path.join(self.outdir, outputFile + "types.h"), 'w') as th:
            template = Template(filename=_get_tpl("types.h.tpl"))
            th.write(template.render(**self.data))
            th.close()
            
        with open(os.path.join(self.outdir, outputFile + "requestfactory.h"), 'w') as fh:
            template = Template(filename=_get_tpl("factory.h.tpl"))
            fh.write(template.render(**self.data))
            fh.close()
    
        with open(os.path.join(self.outdir, outputFile + "requestfactory.cpp"), 'w') as fh:
            template = Template(filename=_get_tpl("factoryjson.cpp.tpl"))
            fh.write(template.render(**self.data))
            fh.close()
            
    def _conversion_method(self, type):
        if type == "QString":
            return "toString"
        elif type == "int":
            return "toInt"
        elif type == "double":
            return "toDouble"
        elif type == "bool":
            return "toBool"
        else:
            return None
            
    def _check(self):
        if not "name" in data:
            raise MicroGenException("\"name\" is a mandatory field")
        if not "module" in data:
            raise MicroGenException("\"module\" is a mandatory field")
        if not "source" in data:
            raise MicroGenException("\"source\" is a mandatory field")
        if data["source"] != "json":
            raise MicroGenException("Only json \"source\" is currently supported")
        
        if not "properties" in data:
            raise MicroGenException("\"properties\" is a mandatory field")
        for property in data["properties"]:
            if not "type" in property:
                raise MicroGenException("For %s \"type\" is a mandatory property field" % repr(property))
            if not "json_path" in property:
                raise MicroGenException("For %s \"json_path\" is a mandatory property field" % repr(property))
            if self._conversion_method(property["type"]) is None:
                raise MicroGenException("For %s, type \"%s\" cannot be handled by JSON" % (repr(property), property["type"]))
    
    def _recursive_build_json_object_tree(self, node, root, leaves):
        if not root in node:
            node[root] = {}
        if len(leaves) > 0:
            self._recursive_build_json_object_tree(node[root], leaves[0], leaves[1:])
       
    def _lower_camel_case(self, name):
        if len(name) == 0:
            return ""
        return name[0].lower() + name[1:]
    
    def _recursive_build_json_object_flat_tree(self, list, name, node):
        for key in node:
            newName = self._lower_camel_case(name + "__" + key)
            list.append((newName, name, key))
            self._recursive_build_json_object_flat_tree(list, newName, node[key])
    
    def _build_json_object_tree(self):
        tree = {}
        for property in data["properties"]:
            path = property["json_path"].split("/")
            if len(path) > 1:
                self._recursive_build_json_object_tree(tree, path[0], path[1:-1])
        object_tree = []
        self._recursive_build_json_object_flat_tree(object_tree, "root", tree)
        data["object_tree"] = object_tree
            
    def _enrich_data(self):
        if data["source"] == "json":
            data["request"] = "::microcore::json::JsonResult"
            data["request_include"] = "<json/jsontypes.h>"
        
    def _enrich_properties(self):
        for property in self.data["properties"]:
            splitted_path = property["json_path"].split("/")
            splitted_path.insert(0, "root")
            property["json_object"] = "__".join(splitted_path[0:-1])
            property["json_key"] = splitted_path[-1]
            property["conversion_method"] = self._conversion_method(property["type"])


def get_data(input):
    f = open(input, 'r')
    returned = yaml.load(f)
    f.close()
    return returned


def generate_factory(data, outdir, outfile):
    # generator = JsonFactoryGenerator(data, input, outdir)
    # generator.render()

    transformer = JsonFactoryTransformer(data)
    transformer.generate()

    generator = JsonFactoryGenerator(transformer.out_data, outdir, outfile)
    generator.generate()


def generate_bean(data, outdir, outfile):
    transformer = BeanTransformer(data)
    transformer.generate()

    generator = BeanGenerator(transformer.out_data, outdir, outfile)
    generator.generate()


def generate_qtbean(data, outdir, outfile):
    transformer = QtBeanTransformer(data)
    transformer.generate()

    generator = QtBeanGenerator(transformer.out_data, outdir, outfile)
    generator.generate()


parser = argparse.ArgumentParser(description='microgen.py: generate boilerplate code for microcore')
parser.add_argument("type", choices=["bean", "qtbean", "factory"], help="type of class to generate.")
parser.add_argument("input", help="input YAML file")
parser.add_argument("outdir", help="output directory")
args = parser.parse_args()

infile = args.input
outdir = args.outdir
outfile = os.path.splitext(os.path.basename(infile))[0]
data = get_data(infile)
if args.type == "bean":
    generate_bean(data, outdir, outfile)
elif args.type == "qtbean":
    generate_bean(data, outdir, outfile)
    generate_qtbean(data, outdir, outfile)
elif args.type == "factory":
    generate_factory(data, outdir, outfile)
