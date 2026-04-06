/* ----------------------------------------------------------------------------
 * Name    : ds_daggerheart.cpp
 * Author  : Rick Cybaniak (Cybr)
 * ----------------------------------------------------------------------------
 * Description:
 *
 * The DAGGERHEART command of DiceServ. See diceserv.cpp for more information
 * about DiceServ, including version and license.
 * ----------------------------------------------------------------------------
 * DaggerHeart is a tabletop RPG by Darrington Press. The Hope and Fear dice
 * mechanic rolls two d12s - one representing Hope, one representing Fear.
 *
 * Results are interpreted as follows:
 *   Hope > Fear  -> with Hope
 *   Fear > Hope  -> with Fear
 *   Hope == Fear -> Critical!
 *
 * The total of both dice is the action result regardless of outcome.
 * ----------------------------------------------------------------------------
 */

#include "diceserv.h"

static ServiceReference<DiceServDataHandlerService> DiceServDataHandler("DiceServDataHandlerService", "DiceServ");

/** DAGGERHEART command
 *
 * Handles the Hope and Fear dice mechanic for the DaggerHeart RPG.
 * Rolls two d12s, one for Hope and one for Fear, and interprets the results.
 */
class DSDaggerHeartCommand : public Command
{
	/** Determine the outcome of a DaggerHeart roll.
	 * @param hope The result of the Hope die
	 * @param fear The result of the Fear die
	 * @return A string describing the outcome
	 */
	static const char *DHOutcome(unsigned hope, unsigned fear)
	{
		if (hope == fear)
			return _("Critical!");
		else if (hope > fear)
			return _("with Hope");
		else
			return _("with Fear");
	}

public:
	DSDaggerHeartCommand(Module *creator) : Command(creator, "diceserv/daggerheart", 0, 2)
	{
		this->AllowUnregistered(true);
		this->RequireUser(true);
		this->SetDesc(_("Rolls Hope and Fear dice for DaggerHeart"));
		this->SetSyntax(_("[[\037channel\037] \037comment\037]"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params) override
	{
		// Insert the dice expression - roll 2d12, one Hope one Fear
		std::vector<Anope::string> newParams = params;
		newParams.insert(newParams.begin() + (source.c ? 1 : 0), "2~1d12");

		DiceServData data;
		data.isExtended = true;
		data.rollPrefix = "DaggerHeart";

		if (!DiceServDataHandler->PreParse(data, source, newParams, 1))
			return;
		if (!DiceServDataHandler->CheckMessageLengthPreProcess(data, source))
			return;

		DiceServDataHandler->Roll(data);

		if (data.errCode != DICE_ERROR_NONE)
		{
			DiceServDataHandler->HandleError(data, source);
			return;
		}

		// data.results[0] is Hope, data.results[1] is Fear
		unsigned hope = static_cast<unsigned>(data.results[0]);
		unsigned fear = static_cast<unsigned>(data.results[1]);
		unsigned total = hope + fear;
		const char *outcome = DHOutcome(hope, fear);

		// Build custom output showing Hope, Fear, total and outcome clearly
		Anope::string output = "<DaggerHeart roll [2d12]: Hope=" + ds_stringify(hope) +
			" Fear=" + ds_stringify(fear) +
			" Total=" + ds_stringify(total) +
			" - " + outcome + ">";

		if (!data.commentStr.empty())
			output += " " + data.commentStr;

		if (!DiceServDataHandler->CheckMessageLengthPostProcess(data, source, output))
		{
			DiceServDataHandler->HandleError(data, source);
			return;
		}
		DiceServDataHandler->SendReply(data, source, output);
	}

	bool OnHelp(CommandSource &source, const Anope::string &) override
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("This command performs the Hope and Fear dice roll for the\n"
			"DaggerHeart tabletop RPG by Darrington Press. It rolls two\n"
			"d12s, one representing Hope and one representing Fear, and\n"
			"interprets the results as follows:\n"
			" \n"
			"    Hope > Fear  - with Hope\n"
			"    Fear > Hope  - with Fear\n"
			"    Hope = Fear  - Critical!\n"
			" \n"
			"The total of both dice is your action result regardless of\n"
			"outcome. The syntax for channel and comment is the same as\n"
			"with the ROLL command (see \002%s%s HELP ROLL\002 for more\n"
			"information).\n"
			" \n"), Config->GetBlock("options").Get<Anope::string>("strictprivmsg", "/").c_str(),
			source.service->nick.c_str());
		const Anope::string &fantasycharacters = Config->GetModule("fantasy").Get<const Anope::string>("fantasycharacter", "!");
		if (!fantasycharacters.empty())
			source.Reply(_("Additionally, if fantasy is enabled, this command can be\n"
				"triggered by using:\n"
				" \n"
				"!daggerheart [\037comment\037]\n"
				" \n"
				"where ! is one of the following characters: %s\n"
				" \n"), fantasycharacters.c_str());
		source.Reply(_("Example:\n"
			"  %s%s DAGGERHEART\n"
			"    <DaggerHeart roll [2d12]: Hope=8 Fear=5 Total=13 - with Hope>\n"
			"  %s%s DAGGERHEART\n"
			"    <DaggerHeart roll [2d12]: Hope=7 Fear=7 Total=14 - Critical!>"),
			Config->GetBlock("options").Get<Anope::string>("strictprivmsg", "/").c_str(),
			source.service->nick.c_str(),
			Config->GetBlock("options").Get<Anope::string>("strictprivmsg", "/").c_str(),
			source.service->nick.c_str());
		return true;
	}
};

class DSDaggerHeart : public Module
{
	DSDaggerHeartCommand cmd;

public:
	DSDaggerHeart(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD), cmd(this)
	{
		this->SetAuthor(DiceServService::Author());
		this->SetVersion(DiceServService::Version());

		if (!DiceServDataHandler)
			throw ModuleException("No interface for DiceServ's data handler");
	}
};

MODULE_INIT(DSDaggerHeart)
