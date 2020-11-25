

template <class VarType>
class ConfigVar
{
  public:
    ConfigVar(VarType& var) : storageLoc(var){};
    void SetNewValue(VarType value) { storageLoc = value; }

  private:
    // Pointer to the actual variable that we will be tweaking if we want
    VarType& storageLoc;
};