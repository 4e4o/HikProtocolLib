#ifndef HIK_ALARM_APPLICATION_HPP
#define HIK_ALARM_APPLICATION_HPP

#include <BaseConfigApplication.h>
#include <boost/signals2.hpp>

class AlarmClient;

class HikAlarmApplication : public BaseConfigApplication {
public:
    HikAlarmApplication(int argc, char* argv[]);
    ~HikAlarmApplication();

protected:
    virtual void onNewClient(AlarmClient*);
    bool start(TConfigItems&) override;
    void doExit() override;

private:
    boost::signals2::signal<void()> m_close;
};

#endif /* HIK_ALARM_APPLICATION_HPP */
