//
// Created by clemens on 26.07.24.
//

#ifndef EMERGENCY_SERVICE_HPP
#define EMERGENCY_SERVICE_HPP

#include <ros/time.h>

#include "services/emergency_service/EmergencyServiceInterface.h"
#include "../../SimRobot.h"

using namespace xbot::service;

class EmergencyService : public EmergencyServiceInterface {  // Corrigé ici
 public:
  explicit EmergencyService(uint16_t service_id, const xbot::serviceif::Context& ctx, const ros::Publisher& publisher, SimRobot& robot)
      : EmergencyServiceInterface(service_id, ctx, publisher), robot_(robot) {
  }

 protected:
  bool OnStart() override;
  void OnStop() override;

 private:
  void tick();
  ManagedSchedule tick_schedule_{scheduler_, IsRunning(), 100'000,
                                 XBOT_FUNCTION_FOR_METHOD(EmergencyService, &EmergencyService::tick, this)};

  SimRobot &robot_;

  ros::Time last_clear_emergency_message_{0};
  std::string emergency_reason{"Boot"};

 protected:
  void OnSetEmergencyChanged(const uint8_t &new_value) override;
};

#endif  // EMERGENCY_SERVICE_HPP
