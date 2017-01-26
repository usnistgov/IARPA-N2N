/**
 * @mainpage
 * @section Overview
 * This is the API for participant-specific one-to-many template generation and
 * template matching algorithms for Intelligence Advanced Research Projects
 * Activity's (IARPA) [2017 Nail to Nail Fingerprint Capture Challenge]
 * (https://www.iarpa.gov/challenges/n2n/n2n.html).  This API is based off the
 * API used for [Fingerprint Vendor Technology Evaluation (FpVTE)
 * 2012](https://www.nist.gov/itl/iad/image-group/ fpvte-2012), by the
 * [National Institute of Standards and Technology (NIST)]
 * (https://www.nist.gov/itl/iad/image-group), released in the public domain.
 *
 * @section Implementation
 * A pure-virtual (abstract) class called N2N::Interface has been created.
 * Participants must implement all methods of N2N::Interface in a subclass, and
 * submit this implementation as a shared library. The name of the library must
 * follow the instructions in N2N::Interface::getIDs(). A test application will
 * link against the submitted library, instantiate an instance of the
 * implementation by calling N2N::Interface::getImplementation(), and perform
 * various template generation and template matching operations.
 *
 * @section Contact
 * Additional information regarding the Nail to Nail Fingerprint Capture
 * Challenge can be obtained by emailing N2NChallenge@iarpa.gov. Additional
 * information regarding this API and the associated software test can be
 * obtained by emailing N2NChallenge@nist.gov.
 *
 * @section License
 * This software was developed at NIST and IARPA by employees of the Federal
 * Government in the course of their official duties. Pursuant to title 17
 * Section 105 of the United States Code, this software is not subject to
 * copyright protection and is in the public domain. NIST and IARPA assume no
 * responsibility whatsoever for its use by other parties, and makes no
 * guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */

#ifndef N2N_H_
#define N2N_H_

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

#include <be_finger.h>
#include <be_image_raw.h>
#include <be_io_recordstore.h>
#include <be_memory_autoarray.h>

/** Contains all the structures and functions used by the API. */
namespace N2N
{
	/** Fingerprint image and image attributes. */
	struct FingerImage
	{
		/** Constructor */
		FingerImage() = default;

		/**
		 * @brief
		 * Constructor.
		 *
		 * @param[in] fgp
		 * Finger position of finger in `rawImage`.
		 * @param[in] imp
		 * Impression type of finger in `rawImage`.
		 * @param[in] nfiq2
		 * NFIQ2 value of `rawImage`.
		 * @param[in] rawImage
		 * Input image data.
		 */
		FingerImage(
		    const BiometricEvaluation::Finger::Position &fgp,
		    const BiometricEvaluation::Finger::Impression &imp,
		    const uint8_t nfiq2,
		    const std::shared_ptr<BiometricEvaluation::Image::Raw>
		    &rawImage) :
		    fgp{fgp},
		    imp{imp},
		    nfiq2{nfiq2},
		    rawImage{rawImage} {}

		/** Finger position of finger in `rawImage`. */
		BiometricEvaluation::Finger::Position fgp{
		    BiometricEvaluation::Finger::Position::Unknown};
		/**
		 * @brief
		 * Impression type of finger in `rawImage`.
		 *
		 * @note
		 * No differentiation is provided between "traditional" and
		 * "participant sensor" rolled impressions.
		 */
		BiometricEvaluation::Finger::Impression imp{
		    BiometricEvaluation::Finger::Impression::Unknown};
		/**
		 * NFIQ2 value.
		 *
		 * | Meaning                  | Value                 |
		 * |:------------------------:|:---------------------:|
		 * | Quality                  | 0 (low) -- 100 (high) |
		 * | Not Calculated           | 254                   |
		 * | Error During Calculation | 255                   |
		 */
		uint8_t nfiq2{254};
		/** Input image data, containing one finger. */
		std::shared_ptr<BiometricEvaluation::Image::Raw> rawImage{};
	};
	/** Convenience type for struct FingerImage. */
	using FingerImage = struct FingerImage;

	/** Object used to report a single candidate in a candidate list. */
	struct Candidate
	{
		/** Constructor. */
		Candidate() = default;

		/**
		 * @brief
		 * Constructor.
		 *
		 * @param[in] templateID
		 * Candidate ID, as provided during finalizeEnrollment().
		 * @param[in] similarity
		 * Similarity of `templateID` to search template.
		 */
		Candidate(
		    const std::string &templateID,
		    double similarity) :
		    templateID{templateID},
		    similarity{similarity} {}

		/**
		 * Candidate's ID, as provided during finalizeEnrollment().
		 * In a candidate list, the empty string represents that no
		 * candidate was found at this position.
		 */
		std::string templateID{};
		/**
		 * Score reflecting similarity between candidate represented
		 * by `templateID` and search template.
		 */
		double similarity{-1};
	};
	/** Convenience type for struct Candidate. */
	using Candidate = struct Candidate;

	/** The status codes that are returned from a function call. */
	enum class StatusCode
	{
		/** Successful completion. */
		Success = 0,
		/** Image size too small or large. */
		ImageSizeNotSupported = 1,
		/** Unsupported template type. */
		TemplateTypeNotSupported = 2,
		/** Could not extract template from image. */
		FailedToExtract = 3,
		/** Could not search enrollment set. */
		FailedToSearch = 4,
		/** Failure to parse data. */
		FailedToParseInput = 5,
		/** There are not enough resources to complete the task. */
		InsufficientResources = 6,
		/** Vendor-defined error. */
		Vendor = 7
	};

	/** Output stream operator for a StatusCode object. */
	inline std::ostream&
	operator<<(
	    std::ostream &s,
	    const StatusCode &sc)
	{
		switch (sc) {
		case StatusCode::Success:
			return (s << "Success");
		case StatusCode::ImageSizeNotSupported:
			return (s << "Image size not supported");
		case StatusCode::TemplateTypeNotSupported:
			return (s << "Template type not supported");
		case StatusCode::FailedToExtract:
			return (s << "Failed to extract");
		case StatusCode::FailedToSearch:
			return (s << "Failed to search");
		case StatusCode::FailedToParseInput:
			return (s << "Failed to parse input");
		case StatusCode::InsufficientResources:
			return (s << "Insufficient resources");
		case StatusCode::Vendor:
			return (s << "Vendor-defined");
		}
	}

	/**
	 * @brief
	 * Information about the completion status of a method.
	 * @details
	 * An object of this class allows the software to return some
	 * information from a method call. The string within this object can
	 * be optionally set to provide more information for debugging. The
	 * status code will be set by the function to Success on success, or
	 * one of the other codes on failure. In failure cases, processing will
	 * proceed with further calls to the function.
	 *
	 * @note
	 * If the SDK encounters a non-recoverable error, an exception should be
	 * thrown and processing will stop.
	 */
	struct ReturnStatus
	{
		/** Constructor. */
		ReturnStatus() = default;

		/**
		 * @brief
		 * Constructor.
		 *
		 * @param[in] code
		 * The return status code.
		 * @param[in] info
		 * The optional information string.
		 */
		ReturnStatus(
		    const StatusCode code,
		    const std::string info) :
		    code{code},
		    info{info} {}

		/** Completion status of the returning method. */
		StatusCode code{StatusCode::Success};
		/** Additional clarifying information about `code`. */
		std::string info{};
	};
	/** Convenience definition of struct ReturnStatus. */
	using ReturnStatus = struct ReturnStatus;

	/** Output stream operator for a ReturnStatus object. */
	inline std::ostream&
	operator<<(
	    std::ostream &s,
	    const ReturnStatus &rs)
	{
		return (s << "Code: " << rs.code << " Info: '" << rs.info <<
		    '"');
	}

	/** Classes of imagery that can be provided. */
	enum class InputType
	{
		/** Images where the subject was present during collection. */
		Capture,
		/** Images where a subject was not present during collection. */
		Latent
	};

	class Interface;
	/**
	 * @brief
	 * The interface to the implementations.
	 * @details
	 * The implementation under test will implement this interface by
	 * subclassing this class and implementing each method.
	 */
	class Interface
	{
	public:
		/**
		 * @brief
		 * Obtain identifying information about the software under test.
		 * @details
		 * Participants will receive an identifier from the project
		 * sponsor, and use this method to hard-code the identifier
		 * into the submission. The information obtained by this method
		 * must form the name of the submitted library, in
		 * the form `libN2N_<identifier>_<revision>.so`.
		 *
		 * @param[out] identifier
		 * The identifier provided to you by the project sponsor.
		 * @param[out] revision
		 * A unique revision number for this submission. No two
		 * submission revision numbers may be the same, and subsequent
		 * submissions should only ever increase this value.
		 * @param[out] email
		 * Point of contact email address.
		 *
		 * @note
		 * This method must return immediately.
		 */
		virtual void
		getIDs(
		    std::string &identifier,
		    uint32_t &revision,
		    std::string &email) = 0;

		/**
		 * @brief
		 * Prepare for calls to makeEnrollmentTemplate().
		 * @details
		 * The function is called once by the testing application
		 * before N >= 1 calls to makeEnrollmentTemplate() on the
		 * current node. The implementation must tolerate execution of
		 * this initialization function and other N >= 1 calls to
		 * makeEnrollmentTemplate() running simultaneously and
		 * independently on the same and/or multiple machines.
		 *
		 * @param[in] configurationDirectory
		 * A read-only directory containing vendor-supplied
		 * configuration parameters or run-time data files.
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * This method must complete with 5 minutes. Reasonable
		 * multithreading is permitted.
		 */
		virtual ReturnStatus
		initMakeEnrollmentTemplate(
		    const std::string &configurationDirectory) = 0;

		/**
		 * @brief
		 * Create an enrollment template for one subject.
		 * @details
		 * This method provides one or more fingerprints from a subject
		 * and tasks the implementation with creating and returning an
		 * object that can represent this subject in an enrollment set.
		 *
		 * @param[in] standardImages
		 * One or more finger images from a single subject.
		 * @param[in] proprietaryImages
		 * One or more proprietary representations of fingers, as
		 * returned from the participant's sensor.
		 * @param[out] enrollmentTemplate
		 * A non-regulated representation of fingers for an enrollment
		 * set.
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * This method should call
		 * BiometricEvaluation::Memory::uint8Array::resize() *before*
		 * any writes to `enrollmentTemplate` to ensure it is large
		 * enough to contain the write. This method should also call
		 * BiometricEvaluation::Memory::uint8Array::resize() *before*
		 * returning so that `enrollmentTemplate` is the exact required
		 * size. All BiometricEvaluation::Memory::uint8Array::size()
		 * bytes of `enrollmentTemplate` will be provided to the
		 * N2N::Interface implementation during finalizeEnrollment().
		 *
		 * @note
		 * 90% of calls to this method must return in three seconds or
		 * less.
		 *
		 * @attention
		 * Multithreading and other multiprocessing techniques are
		 * absolutely not permitted. The testing application will be
		 * calling this method from multiple processes on the same node.
		 */
		virtual ReturnStatus
		makeEnrollmentTemplate(
		    const std::vector<FingerImage> &standardImages,
		    const std::vector<BiometricEvaluation::Memory::uint8Array>
		    &proprietaryImages,
		    BiometricEvaluation::Memory::uint8Array
		    &enrollmentTemplate) = 0;

		/**
		 * @brief
		 * Form an enrollment set from one or more enrollment templates.
		 * @details
		 * This finalization step will prepare the enrolled templates to
		 * be distributed across multiple nodes. The enrollment
		 * directory will then be read-only throughout the duration of
		 * the identification process.
		 *
		 * @param[in] configurationDirectory
		 * A read-only directory containing vendor-supplied
		 * configuration parameters or run-time data files.
		 * @param[in] enrollmentDirectory
		 * The top-level directory in which all enrollment data will
		 * reside. Access permission will be read-write and the
		 * application can populate this directory as needed. The
		 * directory is initially empty. After this method returns, the
		 * directory and its contents will become read-only.
		 * @param[in] nodeCount
		 * The number of nodes the enrollment set will be spread
		 * across.  It is up to the implementation to determine how
		 * best to spread the enrolled templates across the blades in
		 * order to get best performance. If `nodeCount` is not enough
		 * nodes, StatusCode::InsufficientResources should be returned.
		 * @param[in] nodeMemory
		 * Amount of memory available to this process on each node, in
		 * kibibytes.
		 * @param[in] enrollmentTemplates
		 * A read-only RecordStore of enrollment templates, as returned
		 * by makeEnrollmentTemplate().
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * All implementations must be capable of performing searches
		 * using <= 5 nodes. A larger value may be provided for speed,
		 * or a smaller value provided to conserve resources. If a
		 * smaller value is not feasible,
		 * StatusCode::InsufficientResources should
		 * be returned. Implementations that do not return successfully
		 * for values >= 5 will be disqualified.
		 *
		 * @note
		 * The file system does not perform well with the
		 * creation of millions of small files, so the application
		 * should consolidate templates into some sort of database file
		 * within `enrollmentDirectory`.
		 *
		 * @note
		 * This method must return within 90 minutes per 1-million
		 * subjects (e.g., if 5-million enrollment templates are
		 * provided, this method must return within 7.5 hours).
		 *
		 * @note
		 * Reasonable multithreading is permitted. This method will
		 * only be called once.
		 */
		virtual ReturnStatus
		finalizeEnrollment(
		    const std::string &configurationDirectory,
		    const std::string &enrollmentDirectory,
		    const uint8_t nodeCount,
		    const uint64_t nodeMemory,
		    BiometricEvaluation::IO::RecordStore
		    &enrollmentTemplates) = 0;

		/**
		 * @brief
		 * Prepare for calls to makeSearchTemplate().
		 * @details
		 * The function is called once by the testing application before
		 * N >= 1 calls to makeSearchTemplate() on the current node.
		 * The implementation must tolerate execution of this
		 * initialization function and other N >= 1 calls to
		 * makeSearchTemplate() running simultaneously and
		 * independently on the same and/or multiple machines.
		 *
		 * @param[in] configurationDirectory
		 * A read-only directory containing vendor-supplied
		 * configuration parameters or run-time data files.
		 * @param[in] inputType
		 * The types of images that will be provided during all
		 * subsequent calls to makeSearchTemplate().
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * 90% of calls to this method must return in three seconds or
		 * less.
		 *
		 * @note
		 * This method must complete with 5 minutes. Reasonable
		 * multithreading is permitted.
		 */
		virtual ReturnStatus
		initMakeSearchTemplate(
		    const std::string &configurationDirectory,
		    const InputType &inputType) = 0;

		/**
		 * @brief
		 * Create a search template for one subject.
		 * @details
		 * This method provides one or more fingerprints from a subject
		 * and tasks the implementation with creating and returning an
		 * object that can represent this subject as a search initiator.
		 *
		 * @param[in] standardImages
		 * One or more finger images from a single subject.
		 * @param[in] proprietaryImages
		 * One or more proprietary representations of fingers, as
		 * returned from the participant's sensor.
		 * @param[out] searchTemplate
		 * A non-regulated representation of fingers used to initiate
		 * a search.
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * This method should call
		 * BiometricEvaluation::Memory::uint8Array::resize() *before*
		 * any writes to `searchTemplate` to ensure it is large enough
		 * to contain the write. This method should also call
		 * BiometricEvaluation::Memory::uint8Array::resize() *before*
		 * returning so that `searchTemplate` is the exact required
		 * size. All BiometricEvaluation::Memory::uint8Array::size()
		 * bytes of `enrollmentTemplate` will be provided to the
		 * N2N::Interface implementation during finalizeEnrollment().
		 *
		 * @attention
		 * Multithreading and other multiprocessing techniques are
		 * absolutely not permitted. The testing application will be
		 * calling this method from multiple processes on the same node.
		 */
		virtual ReturnStatus
		makeSearchTemplate(
		    const std::vector<FingerImage> &standardImages,
		    const std::vector<BiometricEvaluation::Memory::uint8Array>
		    &proprietaryImages,
		    BiometricEvaluation::Memory::uint8Array
		    &searchTemplate) = 0;

		/**
		 * @brief
		 * Prepare for calls to identifyTemplateStageOne().
		 * @details
		 * The function will be called to initialize each node that
		 * will contain a portion of the enrolled templates. The number
		 * of nodes will be the same as provided in
		 * finalizeEnrollment().
		 *
		 * @param[in] configurationDirectory
		 * A read-only directory containing vendor-supplied
		 * configuration parameters or run-time data files.
		 * @param[in] enrollmentDirectory
		 * The top-level read-only directory in which all finalized
		 * enrollment data resides. The contents of this directory
		 * is identical to the enrollmentDirectory parameter from
		 * finalizeEnrollment(), but the path may not be the same.
		 * @param[in] inputType
		 * The types of images that will be provided during all
		 * subsequent calls to identifyTemplateStageOne().
		 * @param[in] nodeNumber
		 * Node number from nodes in finalizeEnrollment() that is being
		 * initialized. This parameter lets the callee know which
		 * piece of the enrolled templates to load into memory. Nodes
		 * are numbered 0 to (N - 1).
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * This method must complete with 5 minutes. Reasonable
		 * multithreading is permitted.
		 */
		virtual ReturnStatus
		initIdentificationStageOne(
		    const std::string &configurationDirectory,
		    const std::string &enrollmentDirectory,
		    const InputType &inputType,
		    const uint8_t nodeNumber) = 0;

		/**
		 * @brief
		 * Search a template against the partial enrollment set.
		 *
		 * @param[in] searchID
		 * The ID of the search template.  This ID does not identify
		 * subject, but is merely an identifier used to distinguish
		 * different searches performed by the system.  It will be used
		 * as the input to identifyTemplateStageTwo().
		 * @param[in] searchTemplate
		 * A template from makeSearchTemplate().
		 * @param[in] stageOneDataDirectory
		 * This directory will have read-write access. The output
		 * information from identifyTemplateStageOne() that is needed in
		 * identifyTemplateStageTwo() is written in this directory.
		 * This directory will be unique for each search performed.
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * All calls to combined identification functions
		 * (identifyTemplateStageOne() + identifyTemplateStageTwo())
		 * for a single `searchID` must return within 60 seconds for
		 * InputType::Capture and 300 seconds for InputType::Latent. If
		 * identifyTemplateStageOne() takes 55 seconds for searchID XYZ
		 * (InputType::Capture), identifyTemplateStageTwo() must
		 * complete within 5 seconds for the same search ID.
		 *
		 * @attention
		 * Multithreading and other multiprocessing techniques are
		 * absolutely not permitted. The testing application will be
		 * calling this method from multiple processes on the same node.
		 */
		virtual ReturnStatus
		identifyTemplateStageOne(
		    const std::string &searchID,
		    const BiometricEvaluation::Memory::uint8Array
		    &searchTemplate,
		    const std::string &stageOneDataDirectory) = 0;

		/**
		 * @brief
		 * Prepare for calls to identifyTemplateStageTwo().
		 * @details
		 * This second stage of identification uses the results from
		 * identifyTemplateStageOne() to produce a candidate list for
		 * the search subject.
		 *
		 * @param[in] configurationDirectory
		 * A read-only directory containing vendor-supplied
		 * configuration parameters or run-time data files.
		 * @param[in] enrollmentDirectory
		 * The top-level directory in which all finalized enrolled data
		 * resides.  The directory will have read-only access.
		 * @param[in] inputType
		 * The types of images that will be provided during all
		 * subsequent calls to identifyTemplateStageTwo().
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * This method must complete with 5 minutes. Reasonable
		 * multithreading is permitted.
		 */
		virtual ReturnStatus
		initIdentificationStageTwo(
		    const std::string &configurationDirectory,
		    const std::string &enrollmentDirectory,
		    const InputType &inputType) = 0;

		/**
		 * @brief
		 * Produce a candidate list from the results of all calls to
		 * identifyTemplateStageOne() for a particular search ID.
		 * @details
		 * identifyTemplateStageOne() with searchID was called >= 1
		 * times on separate nodes, ideally searching different
		 * subsets of the full enrolled set. In this method, the
		 * implementation should parse the results of the first search
		 * stage to form a final candidate list. This method will only
		 * be called once per searchID and only on a single node.
		 *
		 * @param[in] searchID
		 * The ID of the search template.  This ID does not identify
		 * subject, but is merely an identifier used to distinguish
		 * different searches performed by the system.
		 * @param[in] stageOneDataDirectory
		 * A read-only version of the data generated by
		 * identifyTemplateStageOne().
		 * @param[out] candidates
		 * The candidate list.
		 *
		 * @return
		 * Completion status of the operation.
		 *
		 * @throw BiometricEvaluation::Error::Exception
		 * There was an error processing this request, and the
		 * exception string may contain additional information.
		 *
		 * @note
		 * All calls to combined identification functions
		 * (identifyTemplateStageOne() + identifyTemplateStageTwo())
		 * for a single `searchID` must return within 60 seconds for
		 * InputType::Capture and 300 seconds for InputType::Latent. If
		 * identifyTemplateStageOne() takes 55 seconds for searchID XYZ
		 * (InputType::Capture), identifyTemplateStageTwo() must
		 * complete within 5 seconds for the same search ID.
		 * @note
		 * `candidates` will have `reserve()` called prior to calling
		 * this method.
		 * @note
		 * There shall be [0,100] objects in `candidates` after the
		 * successful return of this method.
		 * @note
		 * `candidates` shall be sorted by descending similarity score
		 * before returning.
		 *
		 * @attention
		 * Multithreading and other multiprocessing techniques are
		 * absolutely not permitted. The testing application will be
		 * calling this method from multiple processes on the same node.
		 */
		virtual ReturnStatus
		identifyTemplateStageTwo(
		    const std::string &searchID,
		    const std::string &stageOneDataDirectory,
		    std::vector<Candidate> &candidates) = 0;

		/**
		 * @brief
		 * Obtain a managed pointer to an implementation of this
		 * interface.
		 *
		 * @return
		 * A managed pointer to the Interface subclass implementation.
		 */
		static std::shared_ptr<Interface>
		getImplementation();

		/** Destructor. */
		virtual ~Interface() = default;
	};
}

#endif	/*  N2N_H_ */
