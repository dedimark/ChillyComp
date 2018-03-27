/*
    Include file for Standerd Rolling functions
        Functions
            RollingStart
            RolliComplete
*/

int    g_iLastPlayerCount   = 0;        //Last count of Players before recheck for new Players

Handle g_hcPlayerCountTimer = INVALID_HANDLE;

RollingStart() {
    DebugLog("Started Rolling");

    int l_iMode = GetConVarInt(g_hcRollMode);

    if (GetConVarInt(g_hcEnabled) == 0) return;

    g_bRollingSequence = true;

    Handle CountedPlayers = GetCountedPlayersArray();

    g_iRollStatus = STATE_ROLLING;

    DebugLog("Rolling Mode is: %d", l_iMode);

    if (l_iMode == 1) {
        SpecRollStart(CountedPlayers);
    } else if (l_iMode == 2) {
        CapRollStart(CountedPlayers);
    } else {
        DebugLog("Unknown Rolling Mode! Please check if mode variable is set properly.");
    }
}

RollingReset() {
    SpecRollReset();
    CapRollReset();

    g_iLastPlayerCount = 0;

    g_iRollStatus = STATE_INITIAL;

    g_hcPlayerCountTimer = CreateTimer(2.00, Timer_CheckPlayerCount, _, TIMER_REPEAT);

    FightReset();
}

public Action:RollingConditions(client, String:arg[]) {
    int l_iMode     = GetConVarInt(g_hcRollMode);
    int l_iTeam     = GetArgTeam(arg);

    DebugLog("Client %d joining team %d", client, l_iTeam);

    if (GetConVarInt(g_hcEnabled) == 0) return Plugin_Continue;

    if(g_iRollStatus >= 1) {
        if(l_iMode == 1) {
            return SpecRollTeamConditions(client, l_iTeam);
        } else if(l_iMode == 2) {
            return CapRollTeamConditions(client, l_iTeam);
        } else {
            return Plugin_Continue;
        }
    }

    return Plugin_Continue;
}

RollingCheckPlayer(client) {
    int l_iMode = GetConVarInt(g_hcRollMode);

    if (l_iMode == 1) {
        SpecRollCheckPlayer(client);
    } else if (l_iMode == 2) {
        CapRollCheckPlayer(client);
    } else {
        DebugLog("Unknown Rolling Mode! Please check if mode variable is set properly.");
    }
}

OnRollingComplete(Handle RolledPlayers) {
    DebugLog("Rolling Complete");

    g_hcPlayerCountTimer = INVALID_HANDLE;

    #if defined DEBUG
        DebugLog("Rolled Player List");
        for(int i = 0; i < GetArraySize(RolledPlayers); i++) {
            int rolled = GetArrayCell(RolledPlayers, i);
            DebugLog("%d - %d", i, rolled);
        }
    #endif

    FightingStart(RolledPlayers);
}

public Action:Timer_CheckPlayerCount(Handle timer) {
    if (GetConVarInt(g_hcEnabled) == 0) return Plugin_Continue;

    if (GetConVarInt(g_hcAutoStart) == 0) return Plugin_Continue;

    int l_iPlayerCounter = CountPlayersInTeam();

    #if defined DEBUG
        if(g_iLastPlayerCount != l_iPlayerCounter) {
            DebugLog("Counter: %d Required: %d", l_iPlayerCounter, GetConVarInt(g_hcTeamSize)*2);
            g_iLastPlayerCount = l_iPlayerCounter;
        }
    #endif

    if (g_iRollStatus == STATE_INITIAL) {
        if(l_iPlayerCounter >= (GetConVarInt(g_hcTeamSize) * 2)) {
            RollingStart();

            return Plugin_Stop;
        }
    }

    return Plugin_Continue;
}

public Handle_RollModeChange(Handle:convar, const String:oldValue[], const String:newValue[]) {
    DebugLog("Rolling mode has changed!");

    RollingReset();
}

public Action:Command_StartRolling(client, args) {
    int l_iAutoStartMode = GetConVarInt(g_hcAutoStart);

    if (l_iAutoStartMode == 1) {
        CPrintToChat(client, "%s[%s] {{matPurple}}Rolling is set to auto enable, you cannot start manually.", COLOR_TAG, TAG);

        return Plugin_Continue;
    }

    if (g_iRollStatus != STATE_INITIAL) {
        CPrintToChat(client, "%s[%s] {{matPurple}}Rolling has already started.", COLOR_TAG, TAG);

        return Plugin_Continue;
    }

    DebugLog("Recieved Command to Start Rolling Process");

    RollingStart();

    return Plugin_Continue;
}

public Action:Command_JoinTeam(client, const String:command[], argc) {
    char arg[128];
    GetCmdArg(1, arg, sizeof(arg));
    return RollingConditions(client, arg);
}

public Action:Command_JoinSpec(client, const String:command[], argc) {
    return RollingConditions(client, "spectate");
}