% for nested_class in classes:
${nested_class["h_nested"]}
% endfor
class ${name}Object : public QObject
{
    Q_OBJECT
    % for property in properties:
    % if property["access"] == "c":
    % if property["type_type"] == "list":
    Q_PROPERTY(QList<${property["qt_type"]}> ${property["name"]} READ ${property["getter"]} CONSTANT)
    % else:
    Q_PROPERTY(${property["qt_type"]} ${property["name"]} READ ${property["getter"]} CONSTANT)
    % endif
    % elif property["access"] == "r" or property["access"] == "rw":
    % if property["type_type"] == "list":
    Q_PROPERTY(QList<${property["qt_type"]}> ${property["name"]} READ ${property["getter"]} NOTIFY ${property["name"]}Changed)
    % else:
    Q_PROPERTY(${property["qt_type"]} ${property["name"]} READ ${property["getter"]} NOTIFY ${property["name"]}Changed)
    % endif
    % endif
    % endfor
public:
    explicit ${name}Object(QObject *parent = nullptr);
    explicit ${name}Object(${nested_name} &&data, QObject *parent = nullptr);
    DISABLE_COPY_DISABLE_MOVE(${name}Object);
    % for property in properties:
    % if property["type_type"] == "list":
    QList<${property["qt_type"]}> ${property["getter"]}() const;
    % else:
    ${property["qt_type"]} ${property["getter"]}() const;
    % endif
    % endfor
    const ${nested_name} & data() const;
    void update(${nested_name} &&data);
% if not const:
Q_SIGNALS:
    % for property in properties:
    % if property["access"] != "c":
    void ${property["name"]}Changed();
    % endif
    % endfor
% endif
private:
    ${nested_name} m_data {};
    % for property in properties:
    % if property["is_qt_object"]:
    % if property["type_type"] == "list":
    QList<${property["qt_class"]} *> m_${property["name"]} {};
    % else:
    ${property["qt_class"]} *m_${property["name"]} {nullptr};
    % endif
    % endif
    % endfor
};
