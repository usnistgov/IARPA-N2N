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

#ifndef N2NV_IDENTSTAGEONE_H_
#define N2NV_IDENTSTAGEONE_H_

namespace N2N
{
	namespace Validation
	{
		/** Methods used to drive calls to finalizeEnrollment() */
		namespace IdentStageOne
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
				/** Number of nodes used during finalization */
				uint8_t numNodes{};
				/** Number of processes per node */
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
			 * Start stage one search processes.
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

			/** fork()ed object that acts as an individual node */
			class NodeWorker : public BE::Process::Worker
			{
			public:
				/**
				 * @brief
				 * Constructor.
				 *
				 * @param[in] nodeNumber
				 * Process number.
				 * @param[in] lib
				 * Shared N2N implementation.
				 * @param[in] args
				 * Arguments from procargs().
				 */
				NodeWorker(
				    uint8_t nodeNumber,
				    const IdentStageOne::Arguments &args);

				/** Default destructor */
				~NodeWorker() = default;

				int32_t
				workerMain()
				    override;
			private:
				/** Shared N2N implementation */
				const std::shared_ptr<N2N::Interface> _lib{};

				/** Arguments from procargs() */
				const Arguments _args;

				/** Node number */
				const uint8_t _nodeNumber;

				/** N2N API convenience wrapper */
				BE::Framework::API<N2N::ReturnStatus> _api{};
			};

			/** fork()ed object that performs stage one searching */
			class ProcessWorker : public BE::Process::Worker
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
				 * @param[in] nodeNumber
				 * Node number. Used to know where to store
				 * output when running multiple "nodes" on a
				 * single node.
				 * @param[in] lib
				 * Shared N2N implementation.
				 * @param[in] args
				 * Arguments from procargs().
				 *
				 * @note
				 * lib->initStageOneIdentification() has been
				 * called.
				 */
				ProcessWorker(
				    uint8_t processNumber,
				    uint8_t nodeNumber,
				    const std::shared_ptr<N2N::Interface> &lib,
				    const IdentStageOne::Arguments &args);

				/** Default destructor */
				~ProcessWorker() = default;

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

#endif /* N2NV_IDENTSTAGEONE_H_ */
