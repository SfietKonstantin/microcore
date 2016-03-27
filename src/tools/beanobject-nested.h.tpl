% for nested_class in classes:
${nested_class["h_nested"]}
% endfor
class ${name}Object : public QObject
{
    Q_OBJECT
    % for property in properties:
    % if property["access"] == "c":
    Q_PROPERTY(${property["qt_type"]} ${property["name"]} READ ${property["getter"]} CONSTANT)
    % elif property["access"] == "r" or property["access"] == "rw":
    Q_PROPERTY(${property["qt_type"]} ${property["name"]} READ ${property["getter"]} NOTIFY ${property["name"]}Changed)
    % endif
    % endfor
public:
    explicit ${name}Object(QObject *parent = nullptr);
    explicit ${name}Object(${nested_name} &&data, QObject *parent = nullptr);
    DISABLE_COPY_DISABLE_MOVE(${name}Object);
    % for property in properties:
    ${property["qt_type"]} ${property["getter"]}() const;
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
    % if property["is_object"]:
    ${property["qt_class"]} *m_${property["name"]} {nullptr};
    % endif
    % endfor
};
