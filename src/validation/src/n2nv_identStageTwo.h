/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) and the Intelligence Advanced Research Projects Activity
 * (IARPA) by employees of the Federal Government in the course of their
 * official duties. Pursuant to title 17 Section 105 of the United States Code,
 * this software is not subject to copyright protection and is in the public
 * domain. NIST and IARPA assume no responsibility whatsoever for its use by
 * other parties, and makes no guarantees, expressed or implied, about its
 * quality, reliability, or any other characteristic.
 */

#include <string>

#include <be_framework_api.h>
#include <be_io_filelogsheet.h>
#include <be_io_recordstore.h>
#include <be_process_forkmanager.h>

#include <n2n.h>

#ifndef N2NV_IDENTSTAGETWO_H_
#define N2NV_IDENTSTAGETWO_H_

namespace N2N
{
	namespace Validation
	{
		/** Methods used to drive calls to finalizeEnrollment() */
		namespace IdentStageTwo
		{
			/** Struct to pass around parsed properties */
			class Arguments
			{
			public:
				/** Path to configuration directory */
				std::string configDir{};
				/** Path to enrollment directory */
				std::string enrollDir{};
				/** Path to root of stage one data */
				std::string stageOneDataRoot{};
				/** Directory where log files are stored*/
				std::string outputDirectory{};
				/** Prefix for log file names */
				std::string prefix{};
				/** Path to RecordStore of search templates */
				std::string searchRSPath{};
				/** Type of input record being searched */
				N2N::InputType searchTemplateType{};
				/** Number of processes */
				uint8_t numProcesses{};
			};

			/**
			 * @brief
			 * Parse command-line arguments.
			 *
			 * @param[in] argc
			 * argc from main().
			 * @param[in] argv
			 * argv from main().
			 *
			 * @return
			 * Parsed arguments.
			 *
			 * @throw
			 * Not all required arguments were set or an invalid
			 * argument was presented.
			 */
			Arguments
			procargs(
			    int argc,
			    char *argv[]);

			/**
			 * @brief
			 * Start stage two search processes.
			 *
			 * @param[in] args
			 * Arguments parsed from procargs().
			 *
			 * @return
			 * Return status to be returned from main().
			 */
			int
			run(
			    const Arguments &args);

			/** fork()ed object that performs stage one searching */
			class Worker : public BE::Process::Worker
			{
			public:
				/** Parameter containing path to log file */
				static const std::string LogPathParam;

				/**
				 * @brief
				 * Constructor.
				 *
				 * @param[in] processNumber
				 * Process number. Used to naively break apart
				 * work.
				 * @param[in] lib
				 * Shared N2N implementation.
				 * @param[in] args
				 * Arguments from procargs().
				 *
				 * @note
				 * lib->initStageTwoIdentification() has been
				 * called.
				 */
				Worker(
				    uint8_t processNumber,
				    const std::shared_ptr<N2N::Interface> &lib,
				    const IdentStageTwo::Arguments &args);

				/** Default destructor */
				~Worker() = default;

				int32_t
				workerMain()
				    override;
			private:
				/** Shared N2N implementation */
				const std::shared_ptr<N2N::Interface> _lib{};

				/** RecordStore of search templates */
				std::shared_ptr<BE::IO::RecordStore> _rs;

				/** Number of searches to perform */
				uint64_t _maxSearches;

				/** Location where _lib writes results */
				const std::string _stageOneDataDir{};

				/** N2N API convenience wrapper */
				BE::Framework::API<N2N::ReturnStatus> _api{};
			};
		}
	}
}

#endif /* N2NV_IDENTSTAGETWO_H_ */
