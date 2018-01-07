/*
============================
HOLY WARS: Utility functions
============================
*/

#include "g_local.h"


// External files:
const char *hw_file_pos_ini_name = "holywars/config/levels.ini";
const char *hw_file_pos_bak_name = "holywars/config/levels.bak";
const char *hw_file_pos_tmp_name = "holywars/config/levels.tmp";

// HWv2.1
// A couple of prototypes:
qboolean IsFemale (edict_t *ent);
edict_t *SelectRandomDeathmatchSpawnPoint (void);
// end HWv2.1


//------------------------------------------------------
// func. void HW_ReportToRespawningPlayer(edict_t *self)
// -------
// A brief message to report the situation to a player.
// ------- 6/3/98 --------------------------------------
// param        : self  The player
// returns      : none
//------------------------------------------------------
void HW_ReportToRespawningPlayer(edict_t *self)
{
        if (hw.halostatus == FREE)
        {
                gi.centerprintf(self, "Find the Halo!\n");
        }
        else if (hw.halostatus == OWNED)
        {
                edict_t *nextplayer = NULL;
                while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
                {
                        if (nextplayer->client->plstatus == SAINT)
                                gi.centerprintf(self, "Kill %s!\n", nextplayer->client->pers.netname);
                }
        }
        else if (hw.halostatus == HEAVEN)
        {
                gi.centerprintf(self, "Go wait for the Halo!\n");
        }
        else
        {
                gi.centerprintf(self, "Run to the Halo spawning point!\n");
        }
}



//-----------------------------------------------------------------
// func. void HW_ReportSaint(edict_t *saint)
// -------
// Reports the name of the Saint to all players (the Saint himself
// gets his own report, too).
// ------- 5/3/98 -------------------------------------------------
// param        : saint  The Saint
// returns      : none
//-----------------------------------------------------------------
void HW_ReportSaint(edict_t *saint)
{
        edict_t *nextplayer = NULL;
        char *sex = NULL;

        while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
        {
                if (nextplayer == saint)
                {
                        gi.centerprintf(nextplayer, "You're still the Saint,\n%s!\n", saint->client->pers.netname);
                }
                else
                {
                        if (IsFemale(saint))
                                sex = "her!";
                        else
                                sex = "him!";

                        gi.centerprintf(nextplayer, "%s is still the Saint.\nKill %s\n", saint->client->pers.netname, sex);
                }
        }
}



//----------------------------------------------------------------
// func. void HW_ReportNewSaint(edict_t *saint)
// -------
// Reports a new sanctification to all players (the Saint himself
// gets his own report, too).
// ------- 8/1/98 ------------------------------------------------
// param        : saint  The Saint
// returns      : none
//----------------------------------------------------------------
void HW_ReportNewSaint(edict_t *saint)
{
        edict_t *nextplayer = NULL;

        while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
        {
                if (nextplayer == saint)
                {
                        gi.centerprintf(nextplayer, "%s, you're a Saint!\n", saint->client->pers.netname);
                }
                else
                {
                        gi.centerprintf(nextplayer, "%s got the Halo!\n", saint->client->pers.netname);
                }
        }
}




//-----------------------------------------------------
// func. void HW_ReportMartyr(edict_t *saint)
// -------
// Reports the name of a dead Saint to all the players.
// Well, how's that for code duplication? :)
// ------- 23/1/98 --------------------------------
// param        : saint  The martyrized Saint
// returns      : none
//-----------------------------------------------------
void HW_ReportMartyr(edict_t *saint)
{
        edict_t *nextplayer = NULL;

        while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
        {
                if (nextplayer == saint)
                {
                        gi.centerprintf(nextplayer, "St. %s, welcome to Heaven!\n", saint->client->pers.netname);
                }
                else
                {
                        gi.centerprintf(nextplayer, "St. %s became a martyr.\n", saint->client->pers.netname);
       
				}
		}
}



//----------------------------------------------------------------
// func. void HW_ReportNewHeretic(edict_t *heretic)
// -------
// Reports a new heretic to all players (the heretic himself
// gets his own report, too).
// ------- 21/3/98 ------------------------------------------------
// param        : saint  The Saint
// returns      : none
//----------------------------------------------------------------
void HW_ReportNewHeretic(edict_t *heretic)
{
        edict_t *nextplayer = NULL;

        while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
        {
                if (nextplayer == heretic)
                {
                        gi.centerprintf(nextplayer, "%s, feel the Wrath of God!\n", heretic->client->pers.netname);
                }
                else
                {
                        gi.centerprintf(nextplayer, "%s is a smelly heretic.\n", heretic->client->pers.netname);
                }
        }
}



//--------------------------------------------------------
// func. int HW_CountEnemies(edict_t *p)
// -------
// Tells how much a player is outnumbered in a deathmatch
// game by counting 1 for every enemy and -1 for each friend.
// subtract one for every teammate.
// ------- 17/1/98 -------------------------------------------------
// param        : p      the player
// returns      : the number of enemies minus the number of friends
//------------------------------------------------------------------
int HW_CountEnemies(edict_t *p)
{
        int total = 0;
        edict_t *nextplayer = NULL;

        while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
        {
                if (p != nextplayer)   // never count self
                {
                        if (OnSameTeam (p, nextplayer))
                                total--;
                        else
                                total++;
                }
        }

        return total;
}

//--------------------------------------------
// func. int HW_GetPlayerAndMediumPing(int *mean_ping);
// -------
// Returns the number of players "really connected".
// IF mean_ping != NULL (points to a variable) this integer will be loaded with
// the mean value of the player's ping
// if there's NO player mean_ping = -1
// there's a check to avoid adding the ping of a player who is sconnecting:
// players MUST have a client and a ping < 990
// ------- 6/4/98 --------------------------
// param        : mean_ping      pointer to an int variable where to store the medium ping
// returns      : number of player
//-------------------------------------------
int HW_GetPlayerAndMediumPing(int *mean_ping)
 {
  int       players    = 0;     // number of players connected (with client AND ping < 990)
  int       pings_sum  = 0;     // sum of connected player's ping
  edict_t  *nextplayer = NULL;  // loop index (for each player...)

     // for each player ....
  while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
        {
         // IF this player has no client or has an HORRIBLE ping
         //     this guy is not connected ... so is NOT a TRUE player
        if(nextplayer->client != NULL && nextplayer->client->ping < 990)
                {       // true player
                players ++;                             // add player
                pings_sum += nextplayer->client->ping;  // add player's ping
                }
        }

       //if mean_ping points to a variable
  if(mean_ping != NULL)
        {       // calc. the mean value
        if(players > 0)
                *mean_ping = pings_sum / players;
        else
                *mean_ping = -1;
        }

  return players;
 }


// HWv2.1 - the next function is new to version 2.1

// here are a bunch of variables and constants used in HW_DeathmatchScoreboardMessage:

const char *HW_score_prompt1 = "    PLAYER   |   |   |SAINTS|HALOS|";
const char *HW_score_prompt2 = "     NAME    |PNT|LAG|KILLED|TAKEN| %";
const char *HW_score_prompt3 = "______________________________________";
const char *HW_score_prompt4 = "             |   |   |      |     |";

char *HW_normal_score_def = "xv %-d yv %-d string2 \"%s\" ";
char *HW_higl_score_def   = "xv %-d yv %-d string \"%s\" ";

//---------------------------------------------------------------------------
// func. void HW_DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
// -------
// Detailed scoreboard message with stats.
//---------------------------------------------------------------------------
void HW_DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// HW 2.1 print the headers
	Com_sprintf (entry, sizeof(entry),
		"xv 1 yv 1 string2 \"%s\" xv 1 yv 10 string2 \"%s\" xv 1 yv 13 string2 \"%s\" ",
		HW_score_prompt1,HW_score_prompt2,HW_score_prompt3);		
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	// hack to align the headers
	Com_sprintf (entry, sizeof(entry),
		"xv 1 yv 4 string2 \"%s\" xv 1 yv 13 string2 \"%s\" ",
		HW_score_prompt4,HW_score_prompt4);		
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	// add the clients in sorted order
	if (total > 12)
		total = 12;

		// for each client
	for (i=0 ; i< total ; i++)
	{
		char HW_Player_name[16], HW_Player_score[64];
		int  ping, dsec_in_play, percent;
		char *score_def;

		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		// add a dogtag
		if (cl_ent == ent || cl_ent == killer)
			score_def = HW_normal_score_def;
		else
			score_def = HW_higl_score_def;

		ping = (cl->ping >= 1000)? 999: cl->ping; 

		dsec_in_play = (level.framenum - cl->resp.enterframe);
		if(dsec_in_play <= 0) 
			percent = 0;
		else
			percent = cl->resp.sanctitytime * 1000 / dsec_in_play;
		
		strncpy(HW_Player_name, cl->pers.netname, 13);
		HW_Player_name[13] = 0;
		sprintf(HW_Player_score, "%-13s %3d %3d %6d %5d %2d%%",
			HW_Player_name, cl->resp.score, ping, 
			cl->resp.killedsaints, cl->resp.takenhalos,
			percent);

		x = 1;
		y = 25 + 10 * i;

	    Com_sprintf (entry, sizeof(entry),
			score_def,
			x, y, HW_Player_score);		

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


// next function is new to HWv2.1
/*
==================
HW_DeathmatchScoreboard

Equivalent of DeathmatchScoreboard for the detailed HW scoreboard.
==================
*/
void HW_DeathmatchScoreboard (edict_t *ent)
{
	HW_DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
======================================
Holy Wars map initialization functions
======================================
*/

/************************************************
        Sintax of the Holy Wars map text file

This is the sintax of the file which HW uses to store the position
of the halo base in each level.

 1) Lines must be less than 512 characters.
 2) The double slash ("//") starts a comment until the next \n
    (C++ sintax).
 3) The loader looks for the following sequence:
        "name" = x1 x2 x3
    Anything else is ignored.

--------- Correct examples (the following segment contains the position of
          the halo for 5 maps - both the map names and the positions are just
                  examples, as these maps don't exist in Quake2):

 "ultrav"  =  3.5  89  102.78
"bdeath" = 1 2 3        I can write junk even without a comment
 I can eve write junk before the sequence... but I cannot use quotation marks
Very good map: "debello" = 54 66 783.3       Adrianaaaaaaaaaaaa !

Some classic map now
 we couldn't miss the good old "DM6"=       634.34        748.73    7598.894
 spaces are no problem for this incredibly intelligent loader...
 "lastone" = 0 0 0

----------end of segment

-------- Wrong examples (a single name is loaded, and it's WRONG)

 "ultrav" = 657 7348
                84378  the sequence of the 3 numbers should have been
                                       on the same line, NOT on different lines
 a comment sequence cuts everything after it  // "djhsalj" 9439 938 93        questo e` un commento

 The following mane is wrong because of the internal space - no spaces
  between the quotes, please
   " wrong" = 15 64 74 // spaces become meaningful when between quotes
 that's enough!
 ------------------

 *******************************************/


//--------------------------------------------
// func. vec3_t *HW_LoadHaloPos(char *map_name)
// -------
// Loads from file hw_file_pos_ini_name the halo position for the specified
// map
// ------- 23/12/97 --------------------------
// param        : map_name      Q2 map to be loaded
// returns      : NULL          no Q2 map reference found
//              : vec3_t        (pnt to static vector) the halo position for map_name
// R static var.: hw_file_pos_ini_name
// W static var.: none
//-------------------------------------------
vec3_t *HW_LoadHaloPos(char *map_name)
 {
 FILE           *f;
 char           *desc_scanf;
 char           buff[512], nome[128];
 static vec3_t  vett;   // this var must be static, as it returns the pointer
 const int      name_size = strlen(map_name);

 if((f=fopen(hw_file_pos_ini_name, "r"))==NULL) return NULL;

 while( fgets( buff, 511, f) != NULL)   // loads a line of text into buff
       {
       char *pnt = strstr(buff, "//");  // looks for comments (//)
       if(pnt != NULL) *pnt = '\0';     // if it finds a comments, it cuts the string
       desc_scanf = (buff[0] == '\"')
                        ? "\"%[^\"]%*[^=]= %f %f %f"
                        : "%*[^\"]\"%[^\"]%*[^=]= %f %f %f";
                    // scanf applied to a string tries to match the 4 variables to load
                        // 2 cases are possible: either it finds the four info and loads them
                        // (returning 4) or it ignores the info
       if( sscanf(buff, desc_scanf , nome, &vett[0], &vett[1], &vett[2]) == 4)
                {
                // found an entry -> verify if it's the one we're looking for
                // printf("nome \"%-s\" =  %6.2f %6.2f %6.2f\n", nome, vett[0], vett[1], vett[2]);
                if(Q_strncasecmp (nome, map_name, name_size) == 0)
                        {
                                // found the right name -> return the pointer
                        fclose(f);
                        return &vett;
                        }
                }
       }
   // finished without finding the name
 fclose(f);
 return NULL;
}



//--------------------------------------------
// func. vec3_t *HW_SaveHaloPos(map_name, position)
// -------
// Save the halo position for the specified map into the file hw_file_pos_ini_name
// ------- 23/12/97 --------------------------
// param        : map_name      Q2 map name
//              : position      x,y,z position for the Halo
// returns      : 0             done
//              : -1            error
// R static var.: hw_file_pos_ini_name
// W static var.: none
//-------------------------------------------
int HW_SaveHaloPos(char *map_name, vec3_t position)
{
 FILE           *f_in, *f_out;
 char           buff[512], nome[128];
 vec3_t         vett;
 int            found = 0;
 const int      name_size = strlen(map_name);

 if((f_in =fopen(hw_file_pos_ini_name, "r"))==NULL) // no r file found -> create a new one
        {
        if((f_out=fopen(hw_file_pos_ini_name, "w+"))==NULL) // fatal error (cannot create file)
                return -1;
        fprintf(f_out, "\"%-s\" = %6.2f %6.2f %6.2f \n",    map_name, position[0],
                                                         position[1], position[2]);
        fclose(f_out);
        return 0;
        }

 if((f_out =fopen(hw_file_pos_tmp_name, "w"))==NULL) // cannot create a file
        {
        fclose(f_in);   // exit (error)
        return -1;
        }

 while( fgets( buff, 511, f_in) != NULL)   // loads a line of text into the buff variable
       {
       if(! found)
                {
                char *pnt = strstr(buff, "//");  // looks for comments (//)

                if(pnt != NULL)
                        *pnt = '\0';     // if it finds a comment, it cuts the string
                                // scanf applied to a string tries to match the 4 variables to load
                                // Two cases are possible:
                                // either it finds the 4 informations and loads them (returns 4)
                                // or it ignores the information
                if( sscanf(buff, "%*[^\"]\"%[^\"]%*[^=]= %f %f %f",
                                nome, &vett[0], &vett[1], &vett[2]) == 4)
                        // found an entry -> check if it's the one we're looking for (found != 0)
                        found = (Q_strncasecmp (nome, map_name, name_size) == 0);
                if(pnt != NULL) *pnt = '/';      // reset the line (with comment)

                if(found)       // executed only if found on this line
                        {       // we must keep the comments and insert the new informations
                            char aux1[512], aux2[512];
                        if( sscanf(buff, "%[^\"]\"%[^\"]%*[^=]= %f %f %f %[^\n]",
                                          aux1, nome, &vett[0], &vett[1], &vett[2], aux2) == 6)
                                        // read ok : insert the new informations

                                fprintf(f_out, "%-s \"%-s\" = %6.2f %6.2f %6.2f %-s\n",
                                         aux1, map_name, position[0], position[1], position[2], aux2);

                        else
                                fprintf(f_out, " \"%-s\" = %6.2f %6.2f %6.2f \n",
                                         map_name, position[0], position[1], position[2]);

                        continue;       // jump to the end of this "while" cycle
                        }
                }
        // this instruction is only executed if the name was NOT found in this iteration
       fputs( buff, f_out);
       }
 fclose(f_in);  // we don't need the read file anymore

 if(! found)    // aargh! we didn't find the name! we must insert it ourselves
        fprintf(f_out, "\n \"%-s\" = %6.2f %6.2f %6.2f \n",
                        map_name, position[0], position[1], position[2]);

 fclose(f_out); // saves on file (we only used buffers until now)

 remove(hw_file_pos_bak_name);                           // if the backup file exists, delete it
 rename(hw_file_pos_ini_name, hw_file_pos_bak_name);     // rename the old file
 if(rename( hw_file_pos_tmp_name, hw_file_pos_ini_name)) // if this one fails, then return error
        return -1;
 else
        return 0;
 // all good things must end
}




//--------------------------------------------
// int HW_BaseInLevel(edict_t *self)
// -------
// Finds a base for the halo. Either it loads it from the levels initialization
// file or it places the base in a random deathmatch player respawn spot.
// ------- 3/2/97 --------------------------
// param        : self  The base entity
// returns      : none
//-------------------------------------------
int HW_BaseInLevel(edict_t *self)
{
        vec3_t *loaded = HW_LoadHaloPos(level.mapname);

        if (loaded != NULL)   // Found a predefined base spot in the level
        {
                VectorCopy((*loaded), (self->s.origin));
                return 1;
        }
        else                  // This is a new level: find a temporary base spot
        {
                edict_t *tempspot;
                tempspot = (edict_t*)SelectRandomDeathmatchSpawnPoint();
                VectorCopy(tempspot->s.origin, self->s.origin);
                (self->s.origin)[2] += 35;   // Raise it a bit
                return 0;
        }
}



//**********************************************************************************
// Dumps all the entities on the console together weith a bunch of interesting data.
//**********************************************************************************
void HW_DumpEntities()
{
        edict_t * ent;

        ent = g_edicts;

		// Also dump some holywars parameters
		gi.dprintf("\n\nSaint sound: %i\n", hw_saintsound->value);   // HWv2.1
		gi.dprintf("\n\nSinner damage: %i\n", hw_sinhurt->value);
		gi.dprintf("\n\nHalo status: %i\n", hw.halostatus);

        gi.dprintf("\n*** Entity list:\n");

        for ( ; ent < &g_edicts[globals.num_edicts] ; ent++)
        {
                gi.dprintf("Entity %i - %s\n", ent->s.number, ent->classname);

                if (ent->inuse)
                        gi.dprintf("In use\n");
                else
                        gi.dprintf("Not in use\n");

                if (ent->touch != NULL)
                        gi.dprintf("Touchable\n");
                else
                        gi.dprintf("Touch = NULL\n");

                if (ent->svflags & SVF_NOCLIENT)
                        gi.dprintf("NoClient\n");
                else
                        gi.dprintf("Client\n");

                gi.dprintf("Solid: %i\n", ent->solid);
                gi.dprintf("SvFlags: %i\n", ent->svflags);

				if (ent->client)
                        gi.dprintf("Plstatus: %i\n", ent->client->plstatus);
        }
}

