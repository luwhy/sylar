#include "../sylar/config.h"
#include "../sylar/log.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/parser.h>
#include <iostream>
sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::Config::Lookup("system.port", (int)8080, "system port");
sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::Config::Lookup("system.value", (float)10.2f, "system value");
sylar::ConfigVar<std::vector<int>>::ptr g_vector_config =
    sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");

sylar::ConfigVar<std::list<int>>::ptr g_list_config =
    sylar::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");

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
    }

    XX(g_vector_config, int_vec, before);
    XX(g_list_config, int_list, before);

    YAML::Node root = YAML::LoadFile("./config/log.yaml");
    sylar::Config::LoadFromYaml(root);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();
    XX(g_vector_config, int_vec, after);
    XX(g_list_config, int_list, after);
#undef XX
}
int main()
{
    // testYaml();
    testConfig();
    return 0;
}