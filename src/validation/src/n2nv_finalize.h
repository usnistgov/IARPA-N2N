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

#include <n2n.h>

#include <string>

#ifndef N2NV_FINALIZE_H_
#define N2NV_FINALIZE_H_

namespace N2N
{
	namespace Validation
	{
		/** Methods used to drive calls to finalizeEnrollment() */
		namespace Finalize
		{
			/** Struct to pass around parsed properties */
			class Arguments
			{
			public:
				/** Path to configuration directory */
				std::string configDir{};
				/** Path to enrollment directory */
				std::string enrollDir{};
				/** Path to enrollment template RecordStore */
				std::string enrollRSPath{};

				/** Kibibytes of RAM per node */
				uint64_t RAMPerNode;
				/** Number of nodes */
				uint8_t numberOfNodes;
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
			 * Start finalization processes.
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
		}
	}
}

#endif /* N2NV_FINALIZE_H_ */
