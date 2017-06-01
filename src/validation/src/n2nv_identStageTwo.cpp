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

#include <sys/stat.h>

#include <cmath>

#include <n2nv_identStageTwo.h>

#include <be_error.h>
#include <be_io_propertiesfile.h>
#include <be_io_utility.h>
#include <be_memory.h>
#include <be_text.h>

using namespace BiometricEvaluation::Framework::Enumeration;

const std::string N2N::Validation::IdentStageTwo::Worker::LogPathParam{
    "_log"};

static std::string
escapeNewlines(
    const std::string &str)
{
	std::string s{str};
	std::string::size_type pos{std::string::npos};
	while ((pos = s.find("\n")) != std::string::npos)
		s.replace(pos, 1, "\\n");
	return (s);
}

N2N::Validation::IdentStageTwo::Arguments
N2N::Validation::IdentStageTwo::procargs(
    int argc,
    char *argv[])
{
	static const std::string ConfigDirKey{"Configuration Directory"};
	static const std::string EnrollDirKey{"Enrollment Directory"};
	static const std::string SearchTemplateTypeKey{"Search Template Type"};
	static const std::string OutputDirKey{"Output Directory"};
	static const std::string PrefixKey{"Prefix"};
	static const std::string NumProcessesKey{"Number of Processes"};
	static const std::string SearchRSPathKey{"Search Template RecordStore"};
	static const std::string StageOneDataRootKey{"Stage One Data Root"};

	static const std::string SearchTemplateTypeValueLatent{"Latent"};
	static const std::string SearchTemplateValueCapture{"Capture"};

	static const std::string NumProcessesDefault{"1"};
	static const std::string PrefixDefault{""};
	static const std::string OutputDirDefault{"."};

	static const std::string usage{"Usage: " + std::string(argv[0]) + " "
	    "<properties.conf>\n\nRequired properties:\n"
	    "\t * " + ConfigDirKey + " = /path/to/directory\n"
	    "\t * " + EnrollDirKey + " = /path/to/directory\n"
	    "\t * " + StageOneDataRootKey + " = /path/to/directory\n"
	    "\t * " + SearchTemplateTypeKey + " = " +
	    SearchTemplateTypeValueLatent + ", " +
	    SearchTemplateValueCapture + "\n"
	    "\t * " + SearchRSPathKey + " = /path/to/RecordStore"
	    "\nOptional properties:\n"
	    "\t * " + NumProcessesKey + " = [1,255] (default: " +
	    NumProcessesDefault + ")\n"
 	    "\t * " + PrefixKey + " = (default: " + PrefixDefault + ")\n"
	    "\t * " + OutputDirKey + " = /path/to/directory (default: " +
	    OutputDirDefault + ")"
	};

	IdentStageTwo::Arguments args{};
	if (argc != 2)
		throw BE::Error::StrategyError(usage);

	std::unique_ptr<BE::IO::PropertiesFile> props;
	try {
		props.reset(new BE::IO::PropertiesFile(
		    argv[1], BE::IO::Mode::ReadOnly, {
		    {NumProcessesKey, NumProcessesDefault}}));
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

	/* Finalized enrollment set directory */
	try {
		args.enrollDir = props->getProperty(EnrollDirKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    EnrollDirKey + '\n' + usage);
	}
	if (!BE::IO::Utility::pathIsDirectory(args.enrollDir))
		throw BE::Error::ObjectDoesNotExist("Cannot find " +
		    EnrollDirKey + ": " + args.enrollDir + '\n' + usage);

	/* Stage one data directory root */
	try {
		args.stageOneDataRoot = props->getProperty(StageOneDataRootKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    StageOneDataRootKey + '\n' + usage);
	}
	if (!BE::IO::Utility::pathIsDirectory(args.stageOneDataRoot))
		throw BE::Error::ObjectDoesNotExist("Cannot find " +
		    StageOneDataRootKey + ": " + args.stageOneDataRoot + '\n' +
		    usage);

	/* Template type */
	try {
		const auto tmplType = props->getProperty(SearchTemplateTypeKey);
		if (BE::Text::caseInsensitiveCompare(tmplType,
		    SearchTemplateTypeValueLatent))
			args.searchTemplateType = InputType::Latent;
		else if (BE::Text::caseInsensitiveCompare(tmplType,
		    SearchTemplateValueCapture))
			args.searchTemplateType = InputType::Capture;
		else
			throw BE::Error::StrategyError("Invalid value for "
			    "property: " + SearchTemplateTypeKey + '\n' +
			    usage);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    ConfigDirKey + '\n' + usage);
	}

	/* Search templates input */
	try {
		args.searchRSPath = props->getProperty(SearchRSPathKey);
	} catch (const BE::Error::ObjectDoesNotExist) {
		throw BE::Error::StrategyError("Missing property: " +
		    SearchRSPathKey + '\n' + usage);
	}
	try {
		BE::IO::RecordStore::openRecordStore(args.searchRSPath);
	} catch (const BE::Error::Exception &e) {
		throw BE::Error::StrategyError("Could not open " +
		    SearchRSPathKey + ": " + args.searchRSPath + " (" +
		    e.whatString() + ")\n" + usage);
	}

	/* Number of processes to launch */
	args.numProcesses = props->getPropertyAsInteger(NumProcessesKey);
	if (args.numProcesses == 0)
		throw BE::Error::StrategyError(NumProcessesKey + " can't be 0");

	args.prefix = props->getProperty(PrefixKey);

	args.outputDirectory = props->getProperty(OutputDirKey);
	if (BE::IO::Utility::makePath(args.outputDirectory, S_IRWXU | S_IRWXG)
	    != 0)
		throw BE::Error::StrategyError("Could not make directory (" +
		    BE::Error::errorStr() + ')');

	return (args);
}

int
N2N::Validation::IdentStageTwo::run(
    const N2N::Validation::IdentStageTwo::Arguments &args)
{
	auto lib = N2N::Interface::getImplementation();

	BE::Framework::API<N2N::ReturnStatus> api;
	api.getWatchdog()->setInterval(
	    10 * 60 * BE::Time::MicrosecondsPerSecond);

	const auto result = api.call([&]() -> N2N::ReturnStatus {
		return (lib->initIdentificationStageTwo(args.configDir,
		    args.enrollDir, args.searchTemplateType));
	});
	if (!result || (result.status.code != StatusCode::Success))
		throw BE::Error::StrategyError("initIdentificationStageTwo "
		    "failed");

	/* Create [1,N] Workers */
	std::vector<std::shared_ptr<BE::Process::WorkerController>> workers;
	BE::Process::ForkManager manager{};
	for (uint8_t i{0}; i < args.numProcesses; ++i) {
		workers.emplace_back(manager.addWorker(
		    std::make_shared<IdentStageTwo::Worker>(i, lib, args)));
		workers.back()->setParameter(IdentStageTwo::Worker::
		    LogPathParam, std::make_shared<std::string>(
		    args.outputDirectory + '/' + args.prefix +
		    std::to_string(i) + ".log" ));
	}

	/* fork and wait */
	try {
		manager.startWorkers();
	} catch (const BE::Error::Exception &e) {
		std::cout << "A node encountered an exception (" <<
		    e.whatString() << ")..." << std::endl;
		return (EXIT_FAILURE);
	}

	for (uint8_t i{0}; i < args.numProcesses; ++i) {
		if (workers.at(i)->getExitStatus() != EXIT_SUCCESS) {
			std::cout << "Node " << std::to_string(i) << " "
			    "did not exit cleanly." << std::endl;
			return (EXIT_FAILURE);
		}
	}

	return (EXIT_SUCCESS);
}

/******************************************************************************/

N2N::Validation::IdentStageTwo::Worker::Worker(
    uint8_t processNumber,
    const std::shared_ptr<N2N::Interface> &lib,
    const IdentStageTwo::Arguments &args) :
    _lib{lib},
    _rs{BE::IO::RecordStore::openRecordStore(args.searchRSPath)},
    _maxSearches{static_cast<uint64_t>(std::ceil(
        this->_rs->getCount() / static_cast<float>(args.numProcesses)))},
    _stageOneDataDir{args.stageOneDataRoot}
{
	if (args.numProcesses > this->_rs->getCount())
		throw BE::Error::StrategyError("Not enough processes for data "
		    "(" + std::to_string(args.numProcesses) + " processes, "
		    "and " + std::to_string(this->_rs->getCount()) + " "
		    "searches in " + this->_rs->getPathname() + ')');

	/* Naive partitioning: sequence ahead */
	const uint64_t stopIndex{processNumber * this->_maxSearches};
	try {
		for (uint64_t i{0}; i < stopIndex; ++i)
			static_cast<void>(this->_rs->sequenceKey());
	} catch (BE::Error::ObjectDoesNotExist) {
		/* Too few records for this many processes */
	}
}

int32_t
N2N::Validation::IdentStageTwo::Worker::workerMain()
{
	std::unique_ptr<BE::IO::FileLogsheet> log;
	try {
		log = BE::Memory::make_unique<BE::IO::FileLogsheet>(
		    this->getParameterAsString(LogPathParam),
		    "EntryType EntryNum SearchID Time APIState RetCode "
		    "Candidates RetInfo");
	} catch (BE::Error::Exception &e) {
		std::cout << "Could not create " +
		    this->getParameterAsString(LogPathParam) + ": " +
		    e.whatString() << std::endl;
		return (EXIT_FAILURE);
	}
	std::string logLine{};
	std::string dataDir{};

	/* Allow 5 minutes maximum per call */
	this->_api.getWatchdog()->setInterval(
	    5 * 60 * BE::Time::MicrosecondsPerSecond);

	for (uint64_t i{0}; i < this->_maxSearches; ++i) {
		/* Get next search template */
		std::string key;
		try {
			key = this->_rs->sequenceKey();
		} catch (BE::Error::ObjectDoesNotExist) {
			break;
		}

		dataDir.clear();
		dataDir = this->_stageOneDataDir + '/' + key;
		chmod(dataDir.c_str(), S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP);

		std::vector<Candidate> candidates;
		candidates.reserve(100);
		const auto result = this->_api.call([&]() -> N2N::ReturnStatus {
			return (this->_lib->identifyTemplateStageTwo(
			    key, dataDir, candidates));
		});

		/* Logging */
		logLine.clear();
		logLine += key + ' ' + std::to_string(result.elapsed) + ' ' +
		    std::to_string(to_int_type(result.currentState)) + ' ';
		if (result) {
			logLine += std::to_string(static_cast<
			    std::underlying_type<N2N::StatusCode>::type>(
			    result.status.code)) + ' ';

			if (candidates.size() == 0)
				logLine += "[<[]>] ";
			else {
				logLine += "[<[";
				for (const auto &candidate : candidates)
					logLine += candidate.templateID + ',' +
					    std::to_string(
					    candidate.similarity) + ';';
				logLine.pop_back();
				logLine += "]>] ";
			}

			logLine += "[<[" +
			    escapeNewlines(result.status.info) + "]>]";
		} else
			logLine += "NA [<[]>] [<[]>]";

		*log << logLine;
		log->newEntry();
	}

	return (EXIT_SUCCESS);
}

/******************************************************************************/

int
main(
    int argc,
    char *argv[])
try {
	using namespace N2N::Validation;
	return (IdentStageTwo::run(IdentStageTwo::procargs(argc, argv)));
} catch (const BE::Error::Exception &e) {
	std::cout << e.what() << std::endl;
	return (EXIT_FAILURE);
}
