from unittest import TestCase
from exception import CheckException
from transformer import Transformer, BeanTransformer, QtBeanTransformer


class TestTransformer(TestCase):
    def test__upper(self):
        self.assertEqual(Transformer._upper("hello"), "Hello")
        self.assertEqual(Transformer._upper("helloWorld"), "HelloWorld")

    def test__make_getter(self):
        self.assertEqual(Transformer._make_getter("bool", "hello"), "isHello")
        self.assertEqual(Transformer._make_getter("int", "hello"), "hello")
        self.assertEqual(Transformer._make_getter("QString", "hello"), "hello")

    def test__make_setter(self):
        self.assertEqual(Transformer._make_setter("hello"), "setHello")

    def test__is_simple_type(self):
        self.assertTrue(Transformer._is_simple_type("bool"))
        self.assertTrue(Transformer._is_simple_type("int"))
        self.assertTrue(Transformer._is_simple_type("double"))
        self.assertFalse(Transformer._is_simple_type("QString"))
        self.assertFalse(Transformer._is_simple_type("std::vector<bool>"))

    def test__make_initial_value(self):
        self.assertEqual(Transformer._make_initial_value("bool"), "false")
        self.assertEqual(Transformer._make_initial_value("int"), "0")
        self.assertEqual(Transformer._make_initial_value("double"), "0.")
        self.assertEqual(Transformer._make_initial_value("QString"), "")
        self.assertEqual(Transformer._make_initial_value("std::vector<bool>"), "")

    def test__check_noname(self):
        transformer = Transformer({})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_nomodule(self):
        transformer = Transformer({"name": "test"})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_noproperties(self):
        transformer = Transformer({"name": "test", "module": "test"})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_propertiesnoname(self):
        properties = [
            {"type": "test", "access": "r"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_propertiesnotype(self):
        properties = [
            {"name": "test", "access": "r"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_propertiesnoaccess(self):
        properties = [
            {"name": "test", "type": "test"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_propertiesbadaccess(self):
        properties = [
            {"name": "test", "type": "test", "access": "a"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_propertiessameproperty(self):
        properties = [
            {"name": "test", "type": "test", "access": "c"},
            {"name": "test", "type": "test", "access": "c"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_propertiesrecursivesameclass(self):
        properties = [
            {"name": "test", "type": "class", "access": "c", "class_name": "Test"},
            {"name": "test2", "type": "class", "access": "c", "class_name": "Test"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        with self.assertRaises(CheckException):
            transformer._check()

    def test__check_properties1(self):
        properties = [
            {"name": "test", "type": "test", "access": "c"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        transformer._check()

    def test__check_properties2(self):
        properties = [
            {"name": "test", "type": "test", "access": "r"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        transformer._check()

    def test__check_properties3(self):
        properties = [
            {"name": "test", "type": "test", "access": "rw"}
        ]
        transformer = Transformer({"name": "test", "module": "test", "properties": properties})
        transformer._check()

    def test__fill(self):
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "access": "rw"
                }
            ]
        }
        out_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "access": "rw"
                }
            ],
            "classes": []
        }
        transformer = Transformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)

    def test_generate(self):
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "access": "rw"
                }
            ]
        }
        out_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "access": "rw"
                }
            ],
            "classes": []
        }
        transformer = Transformer(in_data)
        transformer.generate()
        self.assertDictEqual(transformer.out_data, out_data)

    def test__fill_recursive(self):
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "class_name": "Test",
                    "type": "class",
                    "access": "rw",
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "access": "c"
                        }
                    ]
                }
            ]
        }
        out_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "access": "rw"
                }
            ],
            "classes": [
                {
                    "name": "Test",
                    "module": "module_test",
                    "nested_name": "name_test::Test",
                    "properties": [
                        {
                            "name": "sub_property",
                            "access": "c"
                        }
                    ],
                    "classes": []
                }
            ]
        }
        transformer = Transformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)


class TestBeanTransformer(TestCase):
    def test__fill1(self):
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "access": "rw"
                }
            ]
        }
        out_data = {
            "const": False,
            "includes": ["QString"],
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "nested_type": "QString",
                    "type_type": "object",
                    "access": "rw",
                    "getter": "property",
                    "setter": "setProperty",
                    "initial_value": ""
                }
            ],
            "classes": []
        }
        transformer = BeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)

    def test__fill2(self):
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "access": "c",
                    "list": True
                }
            ]
        }
        out_data = {
            "const": True,
            "includes": ["vector", "QString"],
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "nested_type": "QString",
                    "type_type": "list",
                    "access": "c",
                    "getter": "property",
                    "initial_value": ""
                }
            ],
            "classes": []
        }
        transformer = BeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)

    def test__fill_recursive1(self):
        self.maxDiff = None
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "class_name": "Test",
                    "type": "class",
                    "access": "rw",
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "access": "c"
                        }
                    ]
                }
            ]
        }
        out_data = {
            "name": "name_test",
            "module": "module_test",
            "const": False,
            "includes": ["QString"],
            "properties": [
                {
                    "name": "property",
                    "type": "Test",
                    "nested_type": "name_test::Test",
                    "type_type": "object",
                    "access": "rw",
                    "getter": "property",
                    "setter": "setProperty",
                    "initial_value": ""
                }
            ],
            "classes": [
                {
                    "name": "Test",
                    "module": "module_test",
                    "nested_name": "name_test::Test",
                    "const": True,
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "nested_type": "QString",
                            "type_type": "object",
                            "access": "c",
                            "getter": "sub_property",
                            "initial_value": ""
                        }
                    ],
                    "classes": []
                }
            ]
        }
        transformer = BeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)

    def test__fill_recursive2(self):
        self.maxDiff = None
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "class_name": "Test",
                    "type": "class",
                    "list": True,
                    "access": "rw",
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "access": "c"
                        }
                    ]
                }
            ]
        }
        out_data = {
            "name": "name_test",
            "module": "module_test",
            "const": False,
            "includes": ["vector", "QString"],
            "properties": [
                {
                    "name": "property",
                    "type": "Test",
                    "nested_type": "name_test::Test",
                    "type_type": "list",
                    "access": "rw",
                    "getter": "property",
                    "setter": "setProperty",
                    "initial_value": ""
                }
            ],
            "classes": [
                {
                    "name": "Test",
                    "module": "module_test",
                    "nested_name": "name_test::Test",
                    "const": True,
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "nested_type": "QString",
                            "type_type": "object",
                            "access": "c",
                            "getter": "sub_property",
                            "initial_value": ""
                        }
                    ],
                    "classes": []
                }
            ]
        }
        transformer = BeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)


class TestQtBeanTransformer(TestCase):
    def test__fill1(self):
        self.maxDiff = None
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "access": "rw"
                }
            ]
        }
        out_data = {
            "const": False,
            "includes": ["QString"],
            "name": "name_test",
            "module": "module_test",
            "has_list": False,
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "is_qt_object": False,
                    "qt_type": "QString",
                    "type_type": "object",
                    "nested_type": "QString",
                    "nested_name": "name_test::property",
                    "access": "rw",
                    "getter": "property",
                    "setter": "setProperty",
                    "initial_value": ""
                }
            ],
            "classes": []
        }
        transformer = QtBeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)

    def test__fill2(self):
        self.maxDiff = None
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "access": "c",
                    "list": True
                }
            ]
        }
        out_data = {
            "const": True,
            "includes": ["vector", "QString"],
            "name": "name_test",
            "module": "module_test",
            "has_list": True,
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "is_qt_object": False,
                    "qt_type": "QString",
                    "type_type": "list",
                    "nested_type": "QString",
                    "nested_name": "name_test::property",
                    "access": "c",
                    "getter": "property",
                    "initial_value": ""
                }
            ],
            "classes": []
        }
        transformer = QtBeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)

    def test__fill_recursive1(self):
        self.maxDiff = None
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "class_name": "Test",
                    "type": "class",
                    "access": "rw",
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "access": "c"
                        }
                    ]
                }
            ]
        }
        out_data = {
            "name": "name_test",
            "module": "module_test",
            "const": False,
            "includes": ["QString"],
            "has_list": False,
            "properties": [
                {
                    "name": "property",
                    "type": "Test",
                    "is_qt_object": True,
                    "qt_class": "name_testTestObject",
                    "qt_type": "name_testTestObject *",
                    "type_type": "object",
                    "nested_type": "name_test::Test",
                    "nested_name": "name_test::property",
                    "access": "rw",
                    "getter": "property",
                    "setter": "setProperty",
                    "initial_value": ""
                }
            ],
            "classes": [
                {
                    "name": "name_testTest",
                    "module": "module_test",
                    "nested_name": "name_test::Test",
                    "const": True,
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "is_qt_object": False,
                            "qt_type": "QString",
                            "type_type": "object",
                            "nested_type": "QString",
                            "nested_name": "name_test::Test::sub_property",
                            "access": "c",
                            "getter": "sub_property",
                            "initial_value": ""
                        }
                    ],
                    "classes": []
                }
            ]
        }
        transformer = QtBeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)

    def test__fill_recursive2(self):
        self.maxDiff = None
        in_data = {
            "name": "name_test",
            "module": "module_test",
            "properties": [
                {
                    "name": "property",
                    "class_name": "Test",
                    "type": "class",
                    "access": "rw",
                    "list": True,
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "access": "c"
                        }
                    ]
                }
            ]
        }
        out_data = {
            "name": "name_test",
            "module": "module_test",
            "const": False,
            "includes": ["vector", "QString"],
            "has_list": True,
            "properties": [
                {
                    "name": "property",
                    "type": "Test",
                    "is_qt_object": True,
                    "qt_class": "name_testTestObject",
                    "qt_type": "name_testTestObject *",
                    "type_type": "list",
                    "nested_type": "name_test::Test",
                    "nested_name": "name_test::property",
                    "access": "rw",
                    "getter": "property",
                    "setter": "setProperty",
                    "initial_value": ""
                }
            ],
            "classes": [
                {
                    "name": "name_testTest",
                    "module": "module_test",
                    "nested_name": "name_test::Test",
                    "const": True,
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "is_qt_object": False,
                            "qt_type": "QString",
                            "type_type": "object",
                            "nested_type": "QString",
                            "nested_name": "name_test::Test::sub_property",
                            "access": "c",
                            "getter": "sub_property",
                            "initial_value": ""
                        }
                    ],
                    "classes": []
                }
            ]
        }
        transformer = QtBeanTransformer(in_data)
        transformer._fill()
        self.assertDictEqual(transformer.out_data, out_data)
