/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cmath>
#include <iostream>

#include <be_data_interchange_an2k.h>
#include <be_io_archiverecstore.h>
#include <be_io_propertiesfile.h>
#include <be_text.h>

#include <n2nv_makeTemplates.h>

using namespace BiometricEvaluation::Framework::Enumeration;

const std::string N2N::Validation::MakeTemplates::Worker::ORSPathParam{"_oRS"};
const std::string N2N::Validation::MakeTemplates::Worker::LogPathParam{"_log"};

N2N::Validation::MakeTemplates::Arguments
N2N::Validation::MakeTemplates::procargs(
    int argc,
    char *argv[])
{
	static const std::string ConfigDirKey{"Configuration Directory"};
	static const std::string TemplateTypeKey{"Template Type"};
	static const std::string NumProcessesKey{"Number of Processes"};
	static const std::string PrefixKey{"Prefix"};
	static const std::string OutputDirKey{"Output Directory"};
	static const std::string StandardRSKey{"Standard RecordStore"};
	static const std::string ProprietaryRSKey{"Proprietary RecordStore"};

	static const std::string TemplateTypeValueEnrollment{"Enrollment"};
	static const std::string TemplateTypeValueSearchLatent{"Search Latent"};
	static const std::string TemplateTypeValueSearchCapture{"Search "
	    "Capture"};

	static const std::string NumProcessesDefault{"1"};
	static const std::string OutputDirDefault{"."};
	static const std::string PrefixDefault{""};

	static const std::string usage{"Usage: " + std::string(argv[0]) + " "
	    "<properties.conf>\n\nRequired properties:\n"
	    "\t * " + ConfigDirKey + " = /path/to/directory\n"
	    "\t * " + TemplateTypeKey + " = " + TemplateTypeValueEnrollment +
	    ", " + TemplateTypeValueSearchLatent + ", " +
	    TemplateTypeValueSearchCapture + "\n"
	    "\t * " + StandardRSKey + " = /path/to/RecordStore\n"
	    "\t * " + ProprietaryRSKey + " = /path/to/RecordStore\n"
	    "\nOptional properties:\n"
	    "\t * " + NumProcessesKey + " = [1,255] (default: " +
	    NumProcessesDefault + ")\n"
	    "\t * " + PrefixKey + " = (default: " + PrefixDefault + ")\n"
	    "\t * " + OutputDirKey + " = /path/to/directory (default: " +
	    OutputDirDefault + ")"
	};

	MakeTemplates::Arguments args;
	if (argc != 2)
		throw BE::Error::StrategyError(usage);

	std::unique_ptr<BE::IO::PropertiesFile> props;
	try {
		props.reset(new BE::IO::PropertiesFile(
		    argv[1], BE::IO::Mode::ReadOnly, {
			{NumProcessesKey, NumProcessesDefault},
			{OutputDirKey, OutputDirDefault},
			{PrefixKey, PrefixDefault}
		    }));
	} catch (const BE::Error::Exception &e) {
		throw BE::Error::StrategyError("Could not open \"" +
		    std::string(argv[1]) + "\" (" + e.whatString() + ")");
	}

	/* Configuration directory */
	try {
		args.configDir = props->getProperty(ConfigDirKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    ConfigDirKey + '\n' + usage);
	}

	/* Template type */
	try {
		const auto tmplType = props->getProperty(TemplateTypeKey);
		if (BE::Text::caseInsensitiveCompare(tmplType,
		    TemplateTypeValueEnrollment))
			args.templateType = Type::Enrollment;
		else if (BE::Text::caseInsensitiveCompare(tmplType,
		    TemplateTypeValueSearchLatent))
			args.templateType = Type::SearchLatent;
		else if (BE::Text::caseInsensitiveCompare(tmplType,
		    TemplateTypeValueSearchCapture))
			args.templateType = Type::SearchCapture;
		else
			throw BE::Error::StrategyError("Invalid value for "
			    "property: " + TemplateTypeKey + '\n' + usage);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    ConfigDirKey + '\n' + usage);
	}

	/* Standard imagery input */
	try {
		args.standardRSPath = props->getProperty(StandardRSKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    StandardRSKey + '\n' + usage);
	}

	/* Proprietary imagery input */
	try {
		args.proprietaryRSPath = props->getProperty(ProprietaryRSKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    ProprietaryRSKey + '\n' + usage);
	}

	/* Properties with default values */
	args.numProcesses = props->getPropertyAsInteger(NumProcessesKey);
	if (args.numProcesses == 0)
		throw BE::Error::StrategyError(NumProcessesKey + " can't be 0");
	args.prefix = props->getProperty(PrefixKey);
	args.outputDirectory = props->getProperty(OutputDirKey);
	if (BE::IO::Utility::makePath(args.outputDirectory, S_IRWXU) != 0)
		throw BE::Error::StrategyError("Could not make directory (" +
		    BE::Error::errorStr() + ')');

	return (args);
}

int
N2N::Validation::MakeTemplates::run(
    const N2N::Validation::MakeTemplates::Arguments &args)
{
	/* Initialize pre-fork */
	const auto lib = N2N::Interface::getImplementation();
	switch (args.templateType) {
	case Type::Enrollment:
		lib->initMakeEnrollmentTemplate(args.configDir);
		break;
	case Type::SearchLatent:
		lib->initMakeSearchTemplate(args.configDir, InputType::Latent);
		break;
	case Type::SearchCapture:
		lib->initMakeSearchTemplate(args.configDir, InputType::Capture);
		break;
	}

	/* Create [1,P] Workers */
	BE::Process::ForkManager manager{};
	for (uint8_t i{0}; i < args.numProcesses; ++i) {
		auto worker = manager.addWorker(
		    std::make_shared<MakeTemplates::Worker>(i, lib, args));

		/* Record paths to open after the fork */
		worker->setParameter(Worker::ORSPathParam,
		    std::make_shared<std::string>(args.outputDirectory + '/' +
		    args.prefix + std::to_string(i) + ".rs"));
		worker->setParameter(Worker::LogPathParam,
		    std::make_shared<std::string>(args.outputDirectory + '/' +
		    args.prefix + std::to_string(i) + ".log"));
	}

	/* fork and wait */
	manager.startWorkers();

	return (EXIT_SUCCESS);
}

/******************************************************************************/

N2N::Validation::MakeTemplates::Worker::Worker(
    uint8_t processNumber,
    const std::shared_ptr<N2N::Interface> &lib,
    const MakeTemplates::Arguments &args) :
    BE::Process::Worker::Worker(),
    _lib{lib},
    _templateType{args.templateType},
    _sRS{BE::IO::RecordStore::openRecordStore(args.standardRSPath)},
    _maxRecords{static_cast<uint64_t>(std::ceil(
        _sRS->getCount() / static_cast<float>(args.numProcesses)))}
{
	if (!args.proprietaryRSPath.empty())
		this->_pRS = BE::IO::RecordStore::openRecordStore(
		    args.proprietaryRSPath);
	if (args.numProcesses > _sRS->getCount())
		throw BE::Error::StrategyError("Not enough processes for data");

	/* Naive partitioning: sequence ahead */
	const uint64_t stopIndex{processNumber * this->_maxRecords};
	try {
		for (uint64_t i{0}; i < stopIndex; ++i)
			static_cast<void>(this->_sRS->sequence());
	} catch (BE::Error::ObjectDoesNotExist) {
		/* Too few records for this many processes */
	}
}

int32_t
N2N::Validation::MakeTemplates::Worker::workerMain()
{
	BE::IO::ArchiveRecordStore oRS{this->getParameterAsString(ORSPathParam),
	    ""};
	BE::IO::FileLogsheet log{this->getParameterAsString(LogPathParam),
	    "EntryType EntryNum TemplateID NumStandardInput "
	    "NumProprietaryInput Time TemplateSize APIState RetCode RetInfo"};

	BE::Memory::uint8Array outputTemplate{};
	std::string logLine{};
	/* Naive partitioning requires us to sequence instead of for-range */
	for(uint64_t i{0}; i < this->_maxRecords; ++i) {
		/* Get next subject's imagery (ANSI/NIST-ITL file) */
		BE::IO::RecordStore::Record record;
		try {
			record = this->_sRS->sequence();
		} catch (BE::Error::ObjectDoesNotExist) {
			break;
		}
		const auto standardCaptures = Worker::makeFingerImage(
		    record.data);

		/* Proprietary captures are optional */
		const auto proprietaryCaptures =
		    this->loadProprietaryImages(record.key);

		/* Call template generation method */
		const auto result = this->makeSingleTemplate(standardCaptures,
		    proprietaryCaptures, outputTemplate);

		/* Logging */
		logLine.clear();
		logLine += record.key + ' ' +
		    std::to_string(standardCaptures.size()) + ' ' +
		    std::to_string(proprietaryCaptures.size()) + ' ' +
		    std::to_string(result.elapsed) + ' ' +
		    std::to_string(outputTemplate.size()) + ' ' +
		    std::to_string(static_cast<std::underlying_type<
		    N2N::StatusCode>::type>(result.currentState)) + ' ';
		if (result)
			logLine += std::to_string(static_cast<
			    std::underlying_type<N2N::StatusCode>::type>(
			    result.status.code)) + " [<[" + result.status.info +
			    "]>]";
		else
			logLine += "NA [<[]>]";
		log << logLine;
		log.newEntry();

		/* Write template */
		switch (this->_templateType) {
		case Type::Enrollment:
			/*
			 * "All enrollment templates, regardless of the value of
			 * ReturnStatus, will be provided to the enrollment set
			 * generation step."
			 */
			oRS.insert(record.key, outputTemplate);
			break;
		case Type::SearchLatent:
			/* FALLTHROUGH */
		case Type::SearchCapture:
			/*
			 * "Failures to extract...will not be passed to the
			 * two-stage identification methods."
			 */
			if (result && (result.status.code ==
			    StatusCode::Success))
				oRS.insert(record.key, outputTemplate);
			break;
		}
	}

	return (EXIT_SUCCESS);
}

BiometricEvaluation::Framework::API<N2N::ReturnStatus>::Result
N2N::Validation::MakeTemplates::Worker::makeSingleTemplate(
    const std::vector<N2N::FingerImage> &sIn,
    const std::vector<BE::Memory::uint8Array> &pIn,
    BE::Memory::uint8Array &out)
{
	/* Overwrite previous template and shrink */
	std::fill(out.begin(), out.end(), 0);
	out.resize(0);

	/* Remove this branch from timing */
	std::function<N2N::ReturnStatus(void)> apiFunction;
	switch (this->_templateType) {
	case Type::Enrollment:
		apiFunction = [&]() -> N2N::ReturnStatus {
			return (this->_lib->makeEnrollmentTemplate(sIn, pIn,
			    out));
		};
		break;
	case Type::SearchLatent:
		/* FALLTHROUGH */
	case Type::SearchCapture:
		apiFunction = [&]() -> N2N::ReturnStatus {
			return (this->_lib->makeSearchTemplate(sIn, pIn, out));
		};
		break;
	}

	return (this->_api.call(apiFunction));
}

std::vector<N2N::FingerImage>
N2N::Validation::MakeTemplates::Worker::makeFingerImage(
    const BE::Memory::uint8Array &an2k)
    const
{
	const auto captures = BE::DataInterchange::AN2KRecord{
	    const_cast<BE::Memory::uint8Array&>(an2k)}.getFingerCaptures();

	std::vector<N2N::FingerImage> fingerImages;
	fingerImages.reserve(captures.size());
	for (const auto &c : captures)
		fingerImages.emplace_back(c.getPositions().front(),
		    c.getImpressionType(), /* TODO: NFIQ2 */ 254,
		    std::make_shared<BE::Image::Raw>(
		    BE::Image::Image::getRawImage(c.getImage())));

	return (fingerImages);
}

std::vector<BiometricEvaluation::Memory::uint8Array>
N2N::Validation::MakeTemplates::Worker::loadProprietaryImages(
    const std::string &subjectID)
{
	static constexpr uint8_t MaxFingers{10};
	if (this->_pRS == nullptr)
		return {};

	std::vector<BE::Memory::uint8Array> pData{};
	pData.reserve(MaxFingers);

	for (uint8_t i{0}; i < MaxFingers; ++i) {
		try {
			pData.emplace_back(this->_pRS->read(subjectID + '_' +
			    std::to_string(i)));
		} catch (BE::Error::ObjectDoesNotExist) {}
	}

	pData.shrink_to_fit();
	return (pData);
}

/******************************************************************************/

int
main(
    int argc,
    char *argv[])
try {
	using namespace N2N::Validation;
	return (MakeTemplates::run(MakeTemplates::procargs(argc, argv)));
} catch (const BE::Error::Exception &e) {
	std::cout << e.what() << std::endl;
	return (EXIT_FAILURE);
}
