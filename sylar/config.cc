#include "config.h"
namespace sylar
{
    Config::ConfigVarMap Config::s_datas;

    static void ListAllMember(const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output)
    {
        if (prefix.find_first_not_of("abdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "config invalid name:" << prefix << ":" << node;
            return;
        }
        output.push_back(std::make_pair(prefix, node));
        if (node.IsMap())
        {
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + std::string(".") + it->first.Scalar(), it->second, output);
            }
        }
    }
    void Config::LoadFromYaml(const YAML::Node &root)
    {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        ListAllMember("", root, all_nodes);
        for (auto &i : all_nodes)
        {
            std::string key = i.first;
            if (key.empty())
            {
                continue;
            }
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookupBase(key);
            if (var)
            {
                if (i.second.IsScalar())
                {
                    var->fromString(i.second.Scalar());
                }
                else
                {
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }
    ConfigVarBase::ptr Config::LookupBase(const std::string &name)
    {
        auto it = s_datas.find(name);
        return it == s_datas.end() ? nullptr
                                   : it->second;
    }
} // namespace sylar
