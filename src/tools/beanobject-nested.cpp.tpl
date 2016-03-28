% for nested_class in classes:
${nested_class["c_nested"]}
% endfor
${name}Object::${name}Object(QObject *parent)
    : QObject(parent)
{
    % for property in properties:
    % if property["is_qt_object"]:
    % if not property["type_type"] == "list":
    m_${property["name"]} = new ${property["qt_class"]}(this);
    % endif
    % endif
    % endfor
}

${name}Object::${name}Object(${nested_name} &&data, QObject *parent)
    : QObject(parent), m_data(std::move(data))
{
    % for property in properties:
    % if property["is_qt_object"]:
    % if property["type_type"] == "list":
    for (const ${property["nested_type"]} &value : m_data.${property["getter"]}()) {
        m_${property["name"]}.append(new ${property["qt_class"]}(${property["nested_type"]}(value), this));
    }
    % else:
    m_${property["name"]} = new ${property["qt_class"]}(m_data.${property["getter"]}(), this);
    % endif
    % endif
    % endfor
}

% for property in properties:
% if property["type_type"] == "list":
QList<${property["qt_type"]}> ${name}Object::${property["getter"]}() const
% else:
${property["qt_type"]} ${name}Object::${property["getter"]}() const
% endif
{
    % if property["is_qt_object"]:
    return m_${property["name"]};
    % else:
    return m_data.${property["getter"]}();
    % endif
}

% endfor
const ${nested_name} & ${name}Object::data() const
{
    return m_data;
}

void ${name}Object::update(${nested_name} &&data)
{
% if not const:
    ${nested_name} oldData {m_data};
    m_data = std::move(data);
    % for property in properties:
    % if property["is_qt_object"]:
    % if property["type_type"] == "list":
    qDeleteAll(m_${property["name"]});
    m_${property["name"]}.clear();
    for (const ${property["nested_type"]} &value : m_data.${property["getter"]}()) {
        m_${property["name"]}.append(new ${property["qt_class"]}(${property["nested_type"]}(value), this));
    }
    % else:
    m_${property["name"]}->update(m_data.${property["getter"]}());
    % endif
    % endif
    % endfor
    % for property in properties:
    % if property["access"] != "c":
    if (oldData.${property["getter"]}() != data.${property["getter"]}()) {
        Q_EMIT ${property["name"]}Changed();
    }
    % endif
    % endfor
% else:
    Q_UNUSED(data);
% endif
}
