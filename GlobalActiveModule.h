#ifndef __GlobalActiveModule__H
#define __GlobalActiveModule__H

class GlobalActiveModule{
    public:
      GlobalActiveModule(bool logActive, const char* logName){
          _logActive = logActive;
          _logName = logName;
          _moduleId = moduleId++;
      }
      virtual ~GlobalActiveModule() = default;
      bool getLogActive() { return _logActive; }
      char* getLogName() { return _logName; }
      void setJSONSupport(bool flag){_json_supported = flag;}
      bool isJSONSupported(){return _json_supported;}
      virtual osStatus putMessage(State::Msg *msg) = 0;
      int getNewModuleId() { return moduleId; }
    protected:
      bool _logActive;
      const char* _logName;
      bool _json_supported; 
      int _moduleId;
      inline static int moduleId = 0;
};

#endif