#include "../sylar/config.h"
#include "../sylar/log.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/parser.h>
#include <iostream>
#include <string>
sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::Config::Lookup("system.port", (int)8080, "system port");
sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::Config::Lookup("system.value", (float)10.2f, "system value");
sylar::ConfigVar<std::vector<int>>::ptr g_vector_config =
    sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");

sylar::ConfigVar<std::list<int>>::ptr g_list_config =
    sylar::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");

sylar::ConfigVar<std::set<int>>::ptr g_set_config =
    sylar::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");

sylar::ConfigVar<std::unordered_set<int>>::ptr g_unset_config =
    sylar::Config::Lookup("system.int_unset", std::unordered_set<int>{1, 2}, "system int unset");

sylar::ConfigVar<std::map<std::string, int>>::ptr g_map_config =
    sylar::Config::Lookup("system.int_map", std::map<std::string, int>{{"k", 1}, {"j", 2}}, "system int map");

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr g_unmap_config =
    sylar::Config::Lookup("system.int_unmap", std::unordered_map<std::string, int>{{"k", 1}, {"j", 2}}, "system int unmap");

void print_yaml(const YAML::Node &node, int level)
{
    if (node.IsScalar())
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Tag();
    }
    else if (node.IsNull())
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - " << node.Type() << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void testYaml()
{
    YAML::Node root = YAML::LoadFile("./config/log.yaml");
    print_yaml(root, 0);
}
void testConfig()
{
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();

#define XX(g_var, name, prefix)                                             \
    {                                                                       \
        auto &vx = g_var->getValue();                                       \
        for (auto &i : vx)                                                  \
        {                                                                   \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ":" << i; \
        }                                                                   \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: \n"   \
                                         << g_var->toString();              \
    }

#define XX_M(g_var, name, prefix)                                                                                \
    {                                                                                                            \
        auto &vx = g_var->getValue();                                                                            \
        for (auto &i : vx)                                                                                       \
        {                                                                                                        \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ":{ " << i.first << " - " << i.second << " }"; \
        }                                                                                                        \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: \n"                                        \
                                         << g_var->toString();                                                   \
    }

    XX(g_vector_config, int_vec, before);
    XX(g_list_config, int_list, before);
    XX(g_set_config, int_set, before);
    XX(g_unset_config, int_unset, before);
    XX_M(g_map_config, int_map, before);
    XX_M(g_unmap_config, int_unmap, before);

    YAML::Node root = YAML::LoadFile("./config/test.yaml");
    sylar::Config::LoadFromYaml(root);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();
    XX(g_vector_config, int_vec, after);
    XX(g_list_config, int_list, after);
    XX(g_set_config, int_set, after);
    XX(g_unset_config, int_unset, after);
    XX_M(g_map_config, int_map, after);
    XX_M(g_unmap_config, int_unmap, after);
#undef XX
#undef XX_M
}

class Person
{
public:
    std::string toString() const
    {
        std::stringstream ss;
        ss << "[PersonName=" << m_name << " age=" << m_age << " sex=" << m_sex << "]";
        return ss.str();
    }
    bool operator==(const Person &oth) const
    {
        return m_name == oth.m_name &&
               m_age == oth.m_age && m_sex == oth.m_sex;
    }
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;
};

namespace sylar
{
    template <>
    class LexicalCast<std::string, Person>
    {
    public:
        Person operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            Person p;
            std::stringstream ss;
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<bool>();
            p.m_name = node["age"].as<std::string>();
            return p;
        }
    };
    template <>
    class LexicalCast<Person, std::string>
    {
    public:
        std::string operator()(const Person &p)
        {
            YAML::Node node;
            node["name"] = p.m_name;
            node["age"] = p.m_age;
            node["sex"] = p.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

} // namespace sylar

sylar::ConfigVar<Person>::ptr g_person = sylar::Config::Lookup("class.person", Person(), "system person");

sylar::ConfigVar<std::map<std::string, Person>>::ptr g_person_map = sylar::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");
void testClass()
{
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XXPM(g_var, prefix)                                                                                 \
    {                                                                                                       \
        auto m_p = g_person_map->getValue();                                                                \
        for (auto &i : m_p)                                                                                 \
        {                                                                                                   \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix << ": " << i.first << " - " << i.second.toString(); \
        }                                                                                                   \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix << ": " << m_p.size();                                  \
    }
    g_person->addListener(10, [](const Person &oldValue, const Person &newValue)
                          { SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "old_value = " << oldValue.toString() << " new_value=" << newValue.toString(); });
    // XXPM(g_person_map, "class.map before")
    std::cout << 1 << std::endl;
    YAML::Node root = YAML::LoadFile("./config/test.yaml");
    sylar::Config::LoadFromYaml(root);
    std::cout << 2 << std::endl;
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();

    // XXPM(g_person_map, "class.map after")
#undef XXPM
}

void testLog()
{
    static sylar::Logger::ptr system_log = SYLAR_LOG_NAME("system");
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/lubo/project/sylar/bin/config/log.yaml");
    sylar::Config::LoadFromYaml(root);

    std::cout << "==========" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
}
int main()
{
    // testYaml();
    // testClass();
    testLog();
    return 0;
}