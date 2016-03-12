#!/usr/bin/env python
from mako.template import Template
import yaml
import argparse
import os

def _get_tpl(tpl):
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), tpl)

class MicroGenException(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class BeanGenerator:
    def __init__(self, data, output):
        self.data = data
        self.output = output
                
    def render(self):
        self._check()
        self._check_is_const()
        self._gen_includes()
        self._enrich_properties()
        
        with open(self.output + self.data["name"].lower() + ".h", 'w') as h:
            template = Template(filename=_get_tpl("bean.h.tpl"))
            h.write(template.render(**self.data))
            h.close()
        with open(self.output + self.data["name"].lower() + ".cpp", 'w') as cpp:
            template = Template(filename=_get_tpl("bean.cpp.tpl"))
            cpp.write(template.render(**self.data))
            cpp.close()
        if not "no_qt" in self.data or self.data["no_qt"] == False:
            with open(self.output + self.data["name"].lower() + "object.h", 'w') as qth:
                template = Template(filename=_get_tpl("beanobject.h.tpl"))
                qth.write(template.render(**self.data))
                qth.close()
            with open(self.output + self.data["name"].lower() + "object.cpp", 'w') as qtcpp:
                template = Template(filename=_get_tpl("beanobject.cpp.tpl"))
                qtcpp.write(template.render(**self.data))
                qtcpp.close()
    
    def _check(self):
        if not "name" in data:
            raise MicroGenException("\"name\" is a mandatory field")
        if not "module" in data:
            raise MicroGenException("\"module\" is a mandatory field")
        if not "properties" in data:
            raise MicroGenException("\"properties\" is a mandatory field")
        for property in data["properties"]:
            if not "name" in property:
                raise MicroGenException("For " + repr(property) + " \"name\" is a mandatory property field")
            if not "type" in property:
                raise MicroGenException("For " + repr(property) + " \"type\" is a mandatory property field")
            if not "access" in property:
                raise MicroGenException("For " + repr(property) + " \"access\" is a mandatory property field")
    
    def _is_const(self):
        for property in self.data["properties"]:
            if property["access"] != "c":
                return False
        return True
    
    def _check_is_const(self):
        self.data["const"] = self._is_const()
    
    def _is_qt_type(self, type):
        return type.startswith("Q")
    
    def _add_include(self, type):
        if type not in self.includes:
            self.includes.append(type)
    
    def _gen_includes(self):
        self.includes = []
        for property in self.data["properties"]:
            if self._is_qt_type(property["type"]):
                self._add_include(property["type"])
        self.includes.sort()
        self.data["includes"] = self.includes
        
    def _make_getter(self, type, name):
        if type == "bool":
            return "is" + name[0].upper() + name[1:]
        else:
            return name
    
    def _make_setter(self, name):
        return "set" + name[0].upper() + name[1:]
    
    def _is_simple_type(self, type):
        return type == "int" or type == "double" or type == "bool"
    
    def _make_initial_value(self, type):
        if type == "int":
            return "0"
        elif type == "double":
            return "0."
        elif type == "bool":
            return "false"
        else:
            return ""
    
    def _make_setter_type(self, type):
        if self._is_simple_type(type):
            return type + " "
        else:
            return type + " &&"
    
    def _make_setter_impl(self, type, name):
        if self._is_simple_type(type):
            return name
        else:
            return "std::move(" + name + ")"
    
    def _enrich_properties(self):
        for property in self.data["properties"]:
            property["getter"] = self._make_getter(property["type"], property["name"])
            property["initial_value"] = self._make_initial_value(property["type"])
            property["setter_type"] = self._make_setter_type(property["type"])
            property["setter_impl"] = self._make_setter_impl(property["type"], property["name"])
            if property["access"] == "rw":
                property["setter"] = self._make_setter(property["name"])

class JsonFactoryGenerator:
    def __init__(self, data, output):
        self.data = data
        self.output = output
                
    def render(self):
        self._check()
        self._build_json_object_tree()
        self._enrich_data()
        self._enrich_properties()
        
        with open(self.output + self.data["name"].lower() + "types.h", 'w') as th:
            template = Template(filename=_get_tpl("types.h.tpl"))
            th.write(template.render(**self.data))
            th.close()
            
        with open(self.output + self.data["name"].lower() + "requestfactory.h", 'w') as fh:
            template = Template(filename=_get_tpl("factory.h.tpl"))
            fh.write(template.render(**self.data))
            fh.close()
    
        with open(self.output + self.data["name"].lower() + "requestfactory.cpp", 'w') as fh:
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
                raise MicroGenException("For " + repr(property) + " \"type\" is a mandatory property field")
            if not "json_path" in property:
                raise MicroGenException("For " + repr(property) + " \"json_path\" is a mandatory property field")
            if self._conversion_method(property["type"]) is None:
                raise MicroGenException("For " + repr(property) + ", type \"" + property["type"] + "\" cannot be handled by JSON")
    
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
    data = yaml.load(f)
    f.close()
    return data

def generate_bean(data, output):
    generator = BeanGenerator(data, output)
    generator.render()

def generate_factory(data, output):
    generator = JsonFactoryGenerator(data, output)
    generator.render()

parser = argparse.ArgumentParser(description='microgen.py: generate boilerplate code for microcore')
parser.add_argument("type", choices=["bean", "factory"], help="type of class to generate.")
parser.add_argument("input", help="input YAML file")
args = parser.parse_args()

output = ""
data = get_data(args.input)
if args.type == "bean":
    generate_bean(data, output)
elif args.type == "factory":
    generate_factory(data, output)
