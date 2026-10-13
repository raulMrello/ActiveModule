#ifndef __GlobalActiveModule__H
#define __GlobalActiveModule__H

class GlobalActiveModule{
    public:
      static constexpr const char* ResetCountEnKey = "AppRstEn";
      static constexpr const char* ResetCountKey = "AppRst";
      static constexpr uint32_t ResetCountWindowMs = 60000;

      struct ResetCountDisarmReq {
          uint32_t generation;
          uint8_t fromBootTimer;
      };

      enum ResetCountDisarmStatus : uint8_t {
          ResetCountDisarmStale = 0,
          ResetCountDisarmOk,
          ResetCountDisarmNvsError
      };

      static bool armResetCount(bool startTimer);
      static uint32_t resetCountArmGeneration();
      static ResetCountDisarmStatus disarmResetCountIfCurrent(uint32_t generation, uint32_t* currentGeneration);

      GlobalActiveModule(bool logActive, const char* logName){
          _logActive = logActive;
          _logName = logName;
          _moduleId = moduleId++;
      }
      virtual ~GlobalActiveModule() = default;
      bool getLogActive() { return _logActive; }
      char* getLogName() { return (char*)_logName; }
	  const char* getLogName() const { return _logName; }
	  int getModuleId() const { return _moduleId; }
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