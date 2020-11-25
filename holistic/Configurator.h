#pragma once
#include "ConfigVar.h"
#include <string>
#include <unordered_map>

using VarAdded = std::function<void(std::string)>;

class Configurator
{
  public:
    Configurator(VarAdded addFunc) : varAddedFunc(addFunc){};
    static void StaticInit(VarAdded addFunc) { sInstance.reset(new Configurator(addFunc)); }

    static inline std::unique_ptr<Configurator> sInstance;

    void CreateConfigVar(std::string name, uint32_t& var)
    {
        intVars.emplace(name, new ConfigVar<uint32_t>(var));
        varAddedFunc(name);
    }
    bool SetConfigVar(std::string name, uint32_t var)
    {
        if (intVars.count(name) == 0)
        {
            return false;
        }
        intVars.at(name)->SetNewValue(var);
        return true;
    }

    bool SetConfigVar(std::string name, std::string val)
    {
        if (intVars.count(name) != 0)
        {

            uint32_t valueAsUint = std::stoi(val);
            intVars.at(name)->SetNewValue(valueAsUint);
            return true;
        }

        return false;
    }

  protected:
    std::unordered_map<std::string, std::unique_ptr<ConfigVar<uint32_t>>> intVars;
    VarAdded varAddedFunc;
};
