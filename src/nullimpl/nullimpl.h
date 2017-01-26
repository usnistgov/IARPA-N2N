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

#ifndef NULLIMPL_H_
#define NULLIMPL_H_

#include <n2n.h>

namespace N2N
{
	/** Implementation of N2N::Interface that does essentially nothing. */
	class NullImplementation : public Interface
	{
	public:
		void
		getIDs(
		    std::string &identifier,
		    uint32_t &revision,
		    std::string &email)
		    override;

		ReturnStatus
		initMakeEnrollmentTemplate(
		    const std::string &configurationDirectory)
		    override;

		ReturnStatus
		makeEnrollmentTemplate(
		    const std::vector<FingerImage> &standardImages,
		    const std::vector<BiometricEvaluation::Memory::uint8Array>
		    &proprietaryImages,
		    BiometricEvaluation::Memory::uint8Array &enrollmentTemplate)
		    override;

		ReturnStatus
		finalizeEnrollment(
		    const std::string &configurationDirectory,
		    const std::string &enrollmentDirectory,
		    const uint8_t nodeCount,
		    const uint64_t nodeMemory,
		    BiometricEvaluation::IO::RecordStore &enrollmentTemplates)
		    override;

		ReturnStatus
		initMakeSearchTemplate(
		    const std::string &configurationDirectory,
		    const InputType &inputType)
		    override;

		ReturnStatus
		makeSearchTemplate(
		    const std::vector<FingerImage> &standardImages,
		    const std::vector<BiometricEvaluation::Memory::uint8Array>
		    &proprietaryImages,
		    BiometricEvaluation::Memory::uint8Array &searchTemplate)
		    override;

		ReturnStatus
		initIdentificationStageOne(
		    const std::string &configurationDirectory,
		    const std::string &enrollmentDirectory,
		    const InputType &inputType,
		    const uint8_t nodeNumber)
		    override;

		ReturnStatus
		identifyTemplateStageOne(
		    const std::string &searchID,
		    const BiometricEvaluation::Memory::uint8Array
		    &searchTemplate,
		    const std::string &stageOneDataDirectory)
		    override;

		ReturnStatus
		initIdentificationStageTwo(
		    const std::string &configurationDirectory,
		    const std::string &enrollmentDirectory,
		    const InputType &inputType)
		    override;

		ReturnStatus
		identifyTemplateStageTwo(
		    const std::string &searchID,
		    const std::string &stageOneDataDirectory,
		    std::vector<Candidate> &candidates)
		    override;

		NullImplementation() = default;
		~NullImplementation() = default;

	private:
		/** An object to hold configuration values. */
		struct Configuration
		{
			/** Length of enrollment template */
			uint64_t eLength{};
			/** Length of search template */
			uint64_t sLength{};

			/** Minimum score */
			uint64_t scoreMin{};
			/** Maximum score */
			uint64_t scoreMax{};
		};
		/** Configuration values */
		struct Configuration _config{};

		/** Partitioned enrollment set (read-only) */
		std::shared_ptr<BiometricEvaluation::IO::RecordStore>
		_enrollmentSet;

		/**
		 * @brief
		 * Populate all configuration instance variables from the
		 * configuration file.
		 * @details
		 * `this->_config` is populated with read/default values after
		 * the successful return from this method.
		 *
		 * @param[in] configurationDirectory
		 * Path to the directory containing the configuration file.
		 *
		 * @throw BiometricEvaluation::Error::ObjectDoesNotExist
		 * The configuration file does not exist, or a property is not
		 * present and there is no suitable default specified.
		 */
		void
		loadConfiguration(
		    const std::string &configurationDirectory);

		/**
		 * @brief
		 * Open a partitioned finalized enrollment set.
		 * @details
		 * `this->_enrollmentSet` contains the read-only opened
		 * enrollment set after the successful return of this method.
		 *
		 * @param[in] enrollmentDirectory
		 * Path to the directory containing the enrollment set.
		 * @param[in] nodeNumber
		 * Partition number for the partition to open.
		 *
		 * @throw BiometricEvaluation::Error::ObjectDoesNotExist
		 * The enrollment set does not exist.
		 * @throw BiometricEvaluation::Error::StrategyError
		 * Error opening enrollment exist.
		 */
		void
		openEnrollmentSet(
		    const std::string &enrollmentDirectory,
		    const uint8_t nodeNumber);
	};
}

#endif /* NULLIMPL_H_ */
