/*
Copyright (C) 2007-2008 MDVz0r
Copyright (C) 2018 Joe Reid <joe@joereid.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */


#include "g_local.h"
#include "g_file.h"


qboolean AC_GetRemoteFile(char *bfname) {
    URL_FILE *handle;
    FILE *outf;

    sprintf(buffer, "%s/%s", moddir, ANTICHEATEXCEPTIONLOCALFILE);

    // copy from url line by line with fgets //
    outf = fopen(buffer, "w");
    if (!outf) {
        gi.dprintf("Error opening local anticheat exception file.\n");
        return FALSE;
    }

    handle = url_fopen(bfname, "r");
    if (!handle) {
        gi.dprintf("Error opening remote anticheat exception file.\n");
        fclose(outf);
        return FALSE;
    }

    while (!url_feof(handle)) {
        if (!url_fgets(buffer, sizeof (buffer), handle)) {
            // if it did timeout we are not trying again forever... - hifi
            gi.dprintf("Timeout while waiting for remote AC file.\n");
            url_fclose(handle);
            fclose(outf);
            return FALSE;
        }
        fwrite(buffer, 1, strlen(buffer), outf);
    }

    url_fclose(handle);
    fclose(outf);
    return TRUE;
}


void AC_UpdateList(void) {
    if ((int) q2adminanticheat_enable->value) {
        qboolean ret;
        char cfgAnticheatRemoteList[100];

        if (!q2adminanticheat_file || isBlank(q2adminanticheat_file->string)) {
            q2a_strcpy(cfgAnticheatRemoteList, ANTICHEATEXCEPTIONREMOTEFILE);
        } else {
            q2a_strcpy(cfgAnticheatRemoteList, q2adminanticheat_file->string);
        }

        ret = AC_GetRemoteFile(cfgAnticheatRemoteList);

        if (!ret) {
            gi.dprintf("WARNING: " ANTICHEATEXCEPTIONREMOTEFILE " could not be found\n");
            logEvent(LT_INTERNALWARN, 0, NULL, ANTICHEATEXCEPTIONREMOTEFILE " could not be found", IW_BANSETUPLOAD, 0.0);
			return;
        }
		
		gi.dprintf("Remote anticheat config downloaded\n");
    }
}


void AC_LoadExceptions(void) {	
    if ((int) q2adminanticheat_enable->value) {

        AC_UpdateList();
        // execute exception list even if the download was not succeeded, since there is probably an old version available.
        q2a_strcpy(buffer, "exec " ANTICHEATEXCEPTIONLOCALFILE "\n");
        gi.AddCommandString(buffer);
    }
}


void AC_ReloadExceptions(int startarg, edict_t *ent, int client) {
    AC_LoadExceptions();
    gi.cprintf(ent, PRINT_HIGH, "Exceptionlist loaded.\n");
}
