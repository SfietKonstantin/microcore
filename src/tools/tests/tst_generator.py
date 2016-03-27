from unittest import TestCase
from generator import Generator, BeanGenerator


class TestGenerator(TestCase):
    def test__indent(self):
        self.assertEqual(Generator._indent("hello\nworld"), "    hello\n    world")
        self.assertEqual(Generator._indent("hello\nworld\n"), "    hello\n    world")

    def test__recursive_fill1(self):
        in_data = {
            "name": "name_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "nested_type": "QString",
                    "access": "c",
                    "getter": "property",
                    "setter_type": "QString &&",
                    "setter_impl": "std::move(property)",
                    "initial_value": ""
                }
            ],
            "classes": []
        }
        out_data = {
            "name": "name_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "nested_type": "QString",
                    "access": "c",
                    "getter": "property",
                    "setter_type": "QString &&",
                    "setter_impl": "std::move(property)",
                    "initial_value": "",
                }
            ],
            "classes": []
        }
        generator = BeanGenerator({}, None, None)
        generator._create_templates()
        generator._recursive_fill_templates(in_data)
        self.assertEqual(in_data, out_data)

    def test__recursive_fill2(self):
        self.maxDiff = None
        in_data = {
            "name": "name_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "nested_type": "QString",
                    "access": "c",
                    "getter": "property",
                    "setter_type": "QString &&",
                    "setter_impl": "std::move(property)",
                    "initial_value": ""
                }
            ],
            "classes": [
                {
                    "name": "Test",
                    "nested_name": "name_test::Test",
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "nested_type": "QString",
                            "access": "c",
                            "getter": "sub_property",
                            "setter_type": "QString &&",
                            "setter_impl": "std::move(sub_property)",
                            "initial_value": ""
                        }
                    ],
                    "classes": []
                }
            ]
        }
        h_nested = """    class Test
    {
    public:
        explicit Test() = default;
        explicit Test
        (
            QString &&sub_property
        );
        DEFAULT_COPY_DEFAULT_MOVE(Test);
        QString sub_property() const;
    private:
        QString m_sub_property {};
    };"""
        c_nested = """name_test::Test::Test
(
    QString &&sub_property
)
    : m_sub_property {std::move(sub_property)}
{
}

QString name_test::Test::sub_property() const
{
    return m_sub_property;
}

"""
        out_data = {
            "name": "name_test",
            "properties": [
                {
                    "name": "property",
                    "type": "QString",
                    "nested_type": "QString",
                    "access": "c",
                    "getter": "property",
                    "setter_type": "QString &&",
                    "setter_impl": "std::move(property)",
                    "initial_value": ""
                }
            ],
            "classes": [
                {
                    "name": "Test",
                    "nested_name": "name_test::Test",
                    "h_nested": h_nested,
                    "c_nested": c_nested,
                    "properties": [
                        {
                            "name": "sub_property",
                            "type": "QString",
                            "nested_type": "QString",
                            "access": "c",
                            "getter": "sub_property",
                            "setter_type": "QString &&",
                            "setter_impl": "std::move(sub_property)",
                            "initial_value": ""
                        }
                    ],
                    "classes": []
                }
            ]
        }
        generator = BeanGenerator({}, None, None)
        generator._create_templates()
        generator._recursive_fill_templates(in_data)
        self.assertEqual(in_data, out_data)
