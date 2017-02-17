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
#include <dirent.h>

#include <algorithm>
#include <cstdlib>
#include <cmath>

#include <be_error.h>
#include <be_io_propertiesfile.h>
#include <be_io_recordstore.h>
#include <be_io_utility.h>
#include <be_memory.h>
#include <be_text.h>

#include <nullimpl.h>

namespace BE = BiometricEvaluation;

std::shared_ptr<N2N::Interface>
N2N::Interface::getImplementation()
{
	return (std::make_shared<NullImplementation>());
}

/******************************************************************************/

void
N2N::NullImplementation::getIDs(
    std::string &identifier,
    uint32_t &revision,
    std::string &email)
{
	/* The Makefile uses these values to name the library */
	identifier = "NULLIMPL";
	revision = 1;
	email = "N2NChallenge@nist.gov";
}

N2N::ReturnStatus
N2N::NullImplementation::initMakeEnrollmentTemplate(
    const std::string &configurationDirectory)
{
	/* Load configuration pre-fork */
	this->loadConfiguration(configurationDirectory);
	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::makeEnrollmentTemplate(
    const std::vector<FingerImage> &standardImages,
    const std::vector<BE::Memory::uint8Array> &proprietaryImages,
    BE::Memory::uint8Array &enrollmentTemplate)
{
	/* Fill template with random data */
	enrollmentTemplate.resize(this->_config.eLength *
	    standardImages.size());
	std::generate(enrollmentTemplate.begin(), enrollmentTemplate.end(),
	    std::rand);
	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::finalizeEnrollment(
    const std::string &configurationDirectory,
    const std::string &enrollmentDirectory,
    const uint8_t nodeCount,
    const uint64_t nodeMemory,
    BE::IO::RecordStore &enrollmentTemplates)
{
	/* Ensure availability of sufficient resources */
	constexpr uint64_t tenGiB{1024 * 1024 * 10};
	if (nodeMemory < tenGiB)
		return {StatusCode::InsufficientResources, "< 10 GiB"};
	if (nodeCount == 0)
		return {StatusCode::InsufficientResources, "0 nodes"};

	const uint64_t tmplPerNode = std::ceil(enrollmentTemplates.getCount() /
	    static_cast<float>(nodeCount));
	for (uint8_t n{0}; n < nodeCount; ++n) {
		std::shared_ptr<BE::IO::RecordStore> rs;
		try {
			rs = BE::IO::RecordStore::createRecordStore(
			    enrollmentDirectory + '/' + std::to_string(n),
			    "Finalized enrollment set partition " +
			    std::to_string(n + 1) + '/' +
			    std::to_string(nodeCount),
			    BE::IO::RecordStore::Kind::Default);

			std::for_each(iter, std::next(iter, tmplPerNode),
			    [&](const BE::IO::RecordStore::Record &r) {
				rs->insert(r.key, r.data);
			    });
		} catch (BE::Error::Exception &e) {
			return {StatusCode::Vendor, "Could not create "
			    "enrollment set partition: " + e.whatString()};
		}
	}

	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::initMakeSearchTemplate(
    const std::string &configurationDirectory,
    const InputType &inputType)
{
	/* Load configuration pre-fork */
	this->loadConfiguration(configurationDirectory);
	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::makeSearchTemplate(
    const std::vector<FingerImage> &standardImages,
    const std::vector<BE::Memory::uint8Array> &proprietaryImages,
    BE::Memory::uint8Array &searchTemplate)
{
	/* Fill template with random data */
	searchTemplate.resize(this->_config.sLength * standardImages.size());
	std::generate(searchTemplate.begin(), searchTemplate.end(), std::rand);
	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::initIdentificationStageOne(
    const std::string &configurationDirectory,
    const std::string &enrollmentDirectory,
    const InputType &inputType,
    const uint8_t nodeNumber)
{
	/* Load configuration and open enrollment set pre-fork */
	this->loadConfiguration(configurationDirectory);
	this->openEnrollmentSet(enrollmentDirectory, nodeNumber);

	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::identifyTemplateStageOne(
    const std::string &searchID,
    const BE::Memory::uint8Array &searchTemplate,
    const std::string &stageOneDataDirectory)
{
	static const uint8_t numCandidates{5};

	std::map<std::string, uint64_t> candidates;

	/* Pull off the next N candidates */
	std::string key{};
	for (uint8_t i{0}; i < numCandidates; ++i) {
		try {
			key = this->_enrollmentSet->sequence().key;
		} catch (BE::Error::ObjectDoesNotExist) {
			/* Restart at beginning */
			key = this->_enrollmentSet->sequence(
			    BE::IO::RecordStore::BE_RECSTORE_SEQ_START).key;
		}

		/* Assign a random score */
		candidates[key] = (std::rand() % (this->_config.scoreMax -
		    this->_config.scoreMin)) + this->_config.scoreMin;
	}

	/* Write candidate IDs to a unique filename */
	std::string outputString{};
	for (const auto &c : candidates)
		outputString += c.first + ',' + std::to_string(c.second) + '\n';
	BE::IO::Utility::writeFile((const uint8_t *)outputString.data(),
	    outputString.size(), stageOneDataDirectory + '/' + searchID + '-' +
	    BE::Text::basename(this->_enrollmentSet->getPathname()));

	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::initIdentificationStageTwo(
    const std::string &configurationDirectory,
    const std::string &enrollmentDirectory,
    const InputType &inputType)
{
	return {};
}

N2N::ReturnStatus
N2N::NullImplementation::identifyTemplateStageTwo(
    const std::string &searchID,
    const std::string &stageOneDataDirectory,
    std::vector<Candidate> &candidates)
{
	std::map<std::string, uint64_t> stageOneCandidates;

	std::unique_ptr<DIR, int(*)(DIR*)> dir(::opendir(
	    stageOneDataDirectory.c_str()), closedir);
	if (dir == nullptr)
		return {StatusCode::Vendor, "Could not open "
		    "stageOneDataDirectory (" + stageOneDataDirectory + ") " +
		    BE::Error::errorStr()};

	/* All files in stageOneDataDirectory are CSVs we wrote earlier */
	struct dirent *entry;
	while ((entry = readdir(dir.get())) != nullptr) {
		if (entry->d_ino == 0)
			continue;
		if (BE::Text::caseInsensitiveCompare(entry->d_name, ".") ||
		    BE::Text::caseInsensitiveCompare(entry->d_name, ".."))
			continue;

		const auto file = BE::IO::Utility::readFile(
		    stageOneDataDirectory + '/' + entry->d_name,
		    std::ios_base::in);

		/* Read and parse CSV */
		uint64_t offset = 0, endOffset = 0;
		const uint64_t fileSize = file.size();
		const auto numCandidates = BE::IO::Utility::countLines(file);
		for (uint64_t i{0}; i < numCandidates; ++i) {
			endOffset = offset;
			while ((endOffset < fileSize) &&
			    (file[endOffset] != '\n'))
				++endOffset;

			const auto tokens = BE::Text::split(std::string(
			    file[endOffset], endOffset - offset), ',');
			if (tokens.size() != 2)
				return {StatusCode::Vendor, "Malformed stage "
				    "one data for " + searchID + " in file " +
				    entry->d_name};
			candidates.emplace_back(tokens[0],
			    std::stod(tokens[1]));
		}
	}

	/* Sort candidates by descending similarity score */
	std::stable_sort(candidates.begin(), candidates.end(),
	    [](const Candidate &a, const Candidate &b) {
		return (a.similarity > b.similarity);
	    });

	/* Maximum of 100 candidates */
	if (candidates.size() > 100)
		candidates.erase(std::next(candidates.begin(), 100),
		    candidates.end());

	return {};
}

/******************************************************************************/

void
N2N::NullImplementation::loadConfiguration(
    const std::string &configurationDirectory)
{
	/** Key for length of enrollment template */
	static const std::string ETmplLengthKey{"Enrollment Template Length"};
	/** Key for length of search template (latent + regular) */
	static const std::string STmplLengthKey{"Search Template Length"};
	/** Key for minimum score */
	static const std::string MinScoreKey{"Minimum Score"};
	/* Key for maximum score */
	static const std::string MaxScoreKey{"Maximum Score"};

	/* Derive name of configuration file from library's name */
	uint32_t revision;
	std::string identifier{}, email{};
	this->getIDs(identifier, revision, email);
	std::string ConfigFile{identifier + '-' + std::to_string(revision) +
	    ".conf"};

	/* Default configuration values (used if omitted) */
	static const std::map<const std::string, const std::string> defaults {
	    {ETmplLengthKey, "1024"},
	    {STmplLengthKey, "512"},

	    {MinScoreKey, "0"},
	    {MaxScoreKey, "100"},
	};

	std::unique_ptr<BE::IO::Properties> conf{};
	const std::string confName{configurationDirectory + '/' + ConfigFile};
	try {
		conf = BE::Memory::make_unique<BE::IO::PropertiesFile>(confName,
		    BE::IO::Mode::ReadOnly);
	} catch (BE::Error::Exception &e) {
		throw BE::Error::StrategyError{"Could not load config (" +
		    confName + "): " + e.whatString()};
	}

	this->_config.eLength = conf->getPropertyAsInteger(ETmplLengthKey);
	this->_config.sLength = conf->getPropertyAsInteger(STmplLengthKey);

	this->_config.scoreMin = conf->getPropertyAsInteger(MinScoreKey);
	this->_config.scoreMax = conf->getPropertyAsInteger(MaxScoreKey);
}

void
N2N::NullImplementation::openEnrollmentSet(
    const std::string &enrollmentDirectory,
    const uint8_t nodeNumber)
{
	this->_enrollmentSet = BE::IO::RecordStore::openRecordStore(
	    enrollmentDirectory + '/' + std::to_string(nodeNumber),
	    BE::IO::Mode::ReadOnly);
}
