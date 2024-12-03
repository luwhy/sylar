#ifndef __SYLAR__CONFIG_H__
#define __SYLAR__CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "log.h"
namespace sylar
{
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string &name, const std::string &description = "") : m_name(name),
                                                                                      m_description(description)
        {
        }
        virtual ~ConfigVarBase() {};
        const std::string &getName() const
        {
            return this->m_name;
        }

        const std::string &getDescription() const { return this->m_description; }

        // 转为string
        virtual std::string toString() = 0;

        // 从string转
        virtual bool fromString(const std::string &val) = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };
    template <class T>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ConfigVar(const std::string &name, const T &default_value, const std::string &description) : ConfigVarBase(name, description),
                                                                                                     m_val(default_value)
        {
        }
        std::string toString() override
        {
            try
            {
                boost::lexical_cast<std::string>(m_value);
            }
            catch (std::exception &e)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << "convert:" << typeid(m_value).name() << "to_string";
            }
            return "";
        }
        bool fromString(const std::string &val) override
        {
            try
            {
                m_value = boost::lexical_cast<T>(val);
                return true;
            }
            catch (std::exception &e)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what() << "convert string to" << typeid(m_value).name() << "from string";
            }
            return false;
        }

    private:
        T m_value;
    };

} // namespace sylar

#endif