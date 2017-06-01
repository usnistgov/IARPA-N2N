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

#ifndef N2NV_MAKETEMPLATES_H_
#define N2NV_MAKETEMPLATES_H_

#include <string>
#include <vector>

#include <be_framework_api.h>
#include <be_framework_enumeration.h>
#include <be_io_filelogsheet.h>
#include <be_io_recordstore.h>
#include <be_process_forkmanager.h>

#include <n2n.h>

namespace BE = BiometricEvaluation;

namespace N2N
{
	namespace Validation
	{
		namespace MakeTemplates
		{
			/** The type of template to make */
			enum class Type
			{
				/** Enrollment template */
				Enrollment,
				/** Latent search template */
				SearchLatent,
				/** Capture search template */
				SearchCapture
			};

			/** Struct to pass around parsed properties */
			class Arguments
			{
			public:
				/** Number of processes to spawn */
				uint8_t numProcesses{};
				/** The type of template to make */
				Type templateType{};

				/** Path to RecordStore of standard input */
				std::string standardRSPath{};
				/** Path to RecordStore of proprietary input */
				std::string proprietaryRSPath{};

				/** Path to configuration directory */
				std::string configDir{};
				/** Directory that will contain output */
				std::string outputDirectory{};
				/** File name prefix */
				std::string prefix{};
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
			 * Start template creation processes.
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

			/** fork()ed object that creates templates */
			class Worker : public BE::Process::Worker
			{
			public:
				/** Parameter containing path to output RS */
				static const std::string ORSPathParam;
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
				 * lib->initMake*Template() has been called.
				 */
				Worker(
				    uint8_t processNumber,
				    const std::shared_ptr<N2N::Interface> &lib,
				    const MakeTemplates::Arguments &args);

				/**
				 * @brief
				 * Make one template.
				 *
				 * @param[in] sIn
				 * Standard image data in.
				 * @param[in] pIn
				 * Proprietary image data in
				 * @param[out] out
				 * Template data.
				 *
				 * @return
				 * API result of calling the template creation
				 * method.
				 */
				BiometricEvaluation::Framework::
				API<N2N::ReturnStatus>::Result
				makeSingleTemplate(
				    const std::vector<N2N::FingerImage> &sIn,
				    const std::vector<BE::Memory::uint8Array>
				    &pIn,
				    BE::Memory::uint8Array &out);

				/**
				 * @brief
				 * Populate a N2N::FingerImage given an
				 * ANSI/NIST-ITL file.
				 *
				 * @param[in] an2k
				 * ANSI/NIST-ITL file.
				 *
				 * @return
				 * Collection of N2N::FingerImage
				 * representations of captures in `an2k`.
				 */
				std::vector<N2N::FingerImage>
				makeFingerImage(
				    const BE::Memory::uint8Array &an2k)
				    const;

				/**
				 * @brief
				 * Load all proprietary images for a subject.
				 *
				 * @param[in] subjectID
				 * Subject ID whose proprietary images should be
				 * loaded.
				 *
				 * @return
				 * Vector of [0,10] proprietary images.
				 */
				std::vector<
				BiometricEvaluation::Memory::uint8Array>
				loadProprietaryImages(
				    const std::string &subjectID);

				/** Default destructor */
				~Worker() = default;

				int32_t
				workerMain()
				    override;
			private:
				/** Shared N2N implementation */
				const std::shared_ptr<N2N::Interface> _lib{};

				/** Type of templates to create */
				const Type _templateType{};

				/** RecordStore of standard imagery */
				std::shared_ptr<BE::IO::RecordStore> _sRS;
				/** RecordStore of proprietary imagery */
				std::shared_ptr<BE::IO::RecordStore> _pRS;

				/** Maximum number of records to enroll */
				uint64_t _maxRecords{};

				/** N2N API convenience wrapper */
				BE::Framework::API<N2N::ReturnStatus> _api{};
			};
		}
	}
}

BE_FRAMEWORK_ENUMERATION_DECLARATIONS(
    N2N::Validation::MakeTemplates::Type,
    N2N_Validation_MakeTemplates_Type_EnumToStringMap);

#endif /* N2NV_MAKETEMPLATES_H_ */
