/***********************************************************************************************************************
 *
 * Copyright (c) 2015, ABB Schweiz AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that
 * the following conditions are met:
 *
 *    * Redistributions of source code must retain the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer.
 *    * Redistributions in binary form must reproduce the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer in the documentation
 *      and/or other materials provided with the
 *      distribution.
 *    * Neither the name of ABB nor the names of its
 *      contributors may be used to endorse or promote
 *      products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***********************************************************************************************************************
 */

#ifndef EGM_BASE_INTERFACE_H
#define EGM_BASE_INTERFACE_H

#include <boost/thread.hpp>

#include "egm.pb.h"         // Generated by Google Protocol Buffer compiler protoc
#include "egm_wrapper.pb.h" // Generated by Google Protocol Buffer compiler protoc

#include "egm_common.h"
#include "egm_logger.h"
#include "egm_udp_server.h"

namespace abb
{
namespace egm
{
/**
 * \brief Class for processing asynchronous callbacks.
 *
 * The class provides behavior for:
 * - Processing asynchronous callbacks from a UDP server. The interface recieves the robot controller's
 *   outbound messages and construct inbound messages to the robot controller.
 * - This class can for example be used as a foundation for custom made user interfaces.
 */
class EGMBaseInterface : public AbstractUDPServerInterface
{
public:
  /**
   * \brief A constructor.
   *
   * \param io_service for operating boost asio's asynchronous functions.
   * \param port_number for the server's UDP socket.
   * \param configuration for the interface's configuration.
   */
  EGMBaseInterface(boost::asio::io_service& io_service,
                   const unsigned short port_number,
                   const BaseConfiguration& configuration = BaseConfiguration());

  /**
   * \brief Checks if the underlying server was successfully initialized or not.
   *
   * \return bool indicating if the underlying server was successfully initialized or not.
   */
  bool isInitialized();

  /**
   * \brief Checks if an EGM communication session is connected or not.
   *
   * \return bool indicating if a connection exists between the interface, and the robot controller's EGM client.
   */
  bool isConnected();

  /**
   * \brief Retrieve the most recently received EGM status message.
   *
   * The returned status depends on the EGM communication session(s):
   * - If no session has been active, then an empty status message is returned.
   * - If a session is active, then the most recently received status message is returned.
   * - If any session has been active, then the last status message from the latest session is returned.
   *
   * Note: EGMAct RAPID instructions specifies the frequency of EGM messages, and this affects how often
   *       the status is updated when a communication session is active.
   *
   * \return wrapper::Status containing the most recently received EGM status message.
   */
  wrapper::Status getStatus();

  /**
   * \brief Retrieve the interface's current configuration.
   *
   * \return BaseConfiguration containing the current configurations for the interface.
   */
  BaseConfiguration getConfiguration();

  /**
   * \brief Update the interface's configuration (update is only applied for new EGM communication sessions).
   *
   * \param configuration containing the new configurations for the interface.
   */
  void setConfiguration(const BaseConfiguration& configuration);

protected:
  /**
   * \brief Class for containing inputs from a UDP server.
   */
  class InputContainer
  {
  public:
    /**
     * \brief Default constructor.
     */
    InputContainer();

    /**
     * \brief Parse an array, into an abb::egm::EgmRobot message.
     *
     * \param data containing the serialized array received from the robot controller.
     * \param bytes_transferred for the number of bytes received.
     *
     * \return bool indicating if the parsing was successful or not.
     */
    bool parseFromArray(const char* data, const int bytes_transferred);

    /**
     * \brief Extract the parsed information.
     *
     * \param axes specifying the number of axes of the robot.
     *
     * \return bool indicating if the extraction was successful or not.
     */
    bool extractParsedInformation(const RobotAxes& axes);

    /**
     * \brief Update the previous inputs with the current inputs.
     */
    void updatePrevious();

    /**
     * \brief Retrieve the initial inputs (i.e initial robot controller outputs).
     *
     * \return Input with the initial inputs.
     */
    const wrapper::Input& initial() const { return initial_; };

    /**
     * \brief Retrieve the current inputs (i.e. current robot controller outputs).
     *
     * \return Input with the current inputs.
     */
    const wrapper::Input& current() const { return current_; };

    /**
     * \brief Retrieve the previous inputs (i.e. previous robot controller outputs).
     *
     * \return Input with the previous inputs.
     */
    const wrapper::Input& previous() const { return previous_; };

    /**
     * \brief Retrieve the estimated sample time [s].
     *
     * \return double containing the estimation.
     */
    double estimatedSampleTime() const { return estimated_sample_time_; };

    /**
     * \brief Retrieve a flag, indicating if the received message was the first in a communication session.
     *
     * \return bool indicating if it was the first message or not.
     */
    bool isFirstMessage() const { return first_message_; };

    /**
     * \brief Check if the robot controller's states are ok.
     *
     * I.e. motors are on, RAPID is running and EGM is running.
     *
     * \return bool indicating if the state are ok or not.
     */
    bool statesOk() const;

  private:
    /**
     * \brief Estimate the sample time.
     *
     * \return double containing the estimation.
     */
    double estimateSampleTime();

    /**
     * \brief Estimate the joint and the Cartesian velocities.
     *
     * \return bool indicating if the estimation was successful or not.
     */
    bool estimateAllVelocities();

    /**
     * \brief Container for the "raw" EGM robot message.
     */
    EgmRobot egm_robot_;

    /**
     * \brief Container for the initial inputs, extracted from the EGM robot message.
     */
    wrapper::Input initial_;

    /**
     * \brief Container for the current inputs, extracted from the EGM robot message.
     */
    wrapper::Input current_;

    /**
     * \brief Container for the previous inputs, extracted from the EGM robot message.
     */
    wrapper::Input previous_;

    /**
     * \brief Flag indicating if new data has been received.
     */
    bool has_new_data_;

    /**
     * \brief Flag indicating if the interface's callback has been called before or not.
     */
    bool first_call_;

    /**
     * \brief Flag indicating if the received message was the first in a communication session or not.
     */
    bool first_message_;

    /**
     * \brief The estimated sample time [s].
     */
    double estimated_sample_time_;
  };

  /**
   * \brief Class for containing outputs to a UDP server.
   */
  class OutputContainer
  {
  public:
    /**
     * \brief Default constructor.
     */
    OutputContainer();

    /**
     * \brief Prepare the outputs.
     *
     * \param inputs containing the inputs.
     */
    void prepareOutputs(const InputContainer& inputs);

    /**
     * \brief Generate demo outputs.
     *
     * \param inputs containing the inputs from the robot controller.
     */
    void generateDemoOutputs(const InputContainer& inputs);

    /**
     * \brief Construct the reply string.
     *
     * \param configuration containing the current configurations for the interface.
     */
    void constructReply(const BaseConfiguration& configuration);

    /**
     * \brief Update the previous outputs with the current outputs.
     */
    void updatePrevious();

    /**
     * \brief Retrieve the previous outputs sent to the robot controller.
     *
     * \return Output with the previous outputs.
     */
    const wrapper::Output& previous() const { return previous_; };

    /**
     * \brief Retrieve the current sequence_number.
     *
     * \return unsigned int containing the sequence number.
     */
    unsigned int sequenceNumber() const { return sequence_number_; };

    /**
     * \brief Retrieve the reply string, serialized from the current references.
     *
     * \return string& containing the reply.
     */
    const std::string& reply() const { return reply_; };

    /**
     * \brief Clear the reply content.
     */
    void clearReply() { reply_.clear(); };

    /**
     * \brief Container for the current outputs to send to the robot controller.
     */
    wrapper::Output current;

  private:
    /**
     * \brief Generate demo quaternion outputs.
     *
     * \param inputs containing the inputs from the robot controller.
     * \param t for the interpolation parameter [0 <= t <= 1].
     */
    void generateDemoQuaternions(const InputContainer& inputs, const double t);

    /**
     * \brief Construct the header.
     */
    void constructHeader();

    /**
     * \brief Construct the joint body.
     *
     * \param configuration containing the current configurations for the interface.
     *
     * \return bool indicating if the construction was successful or not.
     */
    bool constructJointBody(const BaseConfiguration& configuration);

    /**
     * \brief Construct the Cartesian body.
     *
     * \param configuration containing the current configurations for the interface.
     *
     * \return bool indicating if the construction was successful or not.
     */
    bool constructCartesianBody(const BaseConfiguration& configuration);

    /**
     * \brief Container for the actual EGM sensor message.
     */
    EgmSensor egm_sensor_;

    /**
     * \brief Container for the previous outputs sent to the robot controller.
     */
    wrapper::Output previous_;

    /**
     * \brief The sequance number, in the current communication session.
     */
    unsigned int sequence_number_;

    /**
     * \brief Container for the reply string.
     */
    std::string reply_;
  };

  /**
   * \brief Struct for containing data regarding an active EGM communication session.
   */
  struct SessionData
  {
    /**
     * \brief Container for the most recently received EGM header message.
     */
    wrapper::Header header;

    /**
     * \brief Container for the most recently received EGM status message.
     */
    wrapper::Status status;

    /**
     * \brief Mutex for protecting the session data.
     */
    boost::mutex mutex;
  };

  /**
   * \brief Struct for containing the base configuration data.
   */
  struct BaseConfigurationContainer
  {
    /**
     * \brief A constructor.
     *
     * \param initial specifying the initial configuration.
     */
    BaseConfigurationContainer(const BaseConfiguration& initial)
    :
    active(initial),
    update(initial),
    has_pending_update(false)
    {}

    /**
     * \brief The active configuration.
     */
    BaseConfiguration active;

    /**
     * \brief The configuration update.
     */
    BaseConfiguration update;

    /**
     * \brief Flag indicating if the active configuration should be updated.
     */
    bool has_pending_update;

    /**
     * \brief Mutex for protecting the configuration.
     */
    boost::mutex mutex;
  };

  /**
   * \brief Log input, from robot controller, and output, to robot controller, into a CSV file.
   *
   * \param inputs containing the inputs from the robot controller.
   * \param outputs containing the outputs to the robot controller.
   * \param max_time specifying the max amount of time to log.
   */
  void logData(const InputContainer& inputs, const OutputContainer& outputs, const double max_time);

  /**
   * \brief Initialize the callback.
   *
   * \param server_data containing the UDP server's callback data.
   *
   * \return bool indicating if the initialization succeeded or not.
   */
  bool initializeCallback(const UDPServerData& server_data);

  /**
   * \brief Static constant wait time [ms] used when determining if a connection has been established or not.
   *
   * I.e. a connection between the interface's UDP server, and a robot controller's EGM client.
   */
  static const unsigned int WAIT_TIME_MS = 100;

  /**
   * \brief Container for the inputs, to the interface, from the UDP server.
   */
  InputContainer inputs_;

  /**
   * \brief Container for the outputs, from the interface, to the UDP server.
   */
  OutputContainer outputs_;

  /**
   * \brief Container for session data (most recently received header and status messages).
   */
  SessionData session_data_;

  /**
   * \brief Logger, for logging EGM messages to a CSV file.
   */
  boost::shared_ptr<EGMLogger> p_logger_;

  /**
   * \brief The interface's configuration.
   */
  BaseConfigurationContainer configuration_;

  /**
   * \brief Server for managing the communication with the robot controller.
   */
  UDPServer udp_server_;

private:
  /**
   * \brief Handle callback requests from an UDP server.
   *
   * \param server_data containing the UDP server's callback data.
   *
   * \return string& containing the reply.
   */
  const std::string& callback(const UDPServerData& server_data);
};

} // end namespace egm
} // end namespace abb

#endif // EGM_BASE_INTERFACE_H
