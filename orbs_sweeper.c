#define _WIN32_WINNT 0x0A00
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <tlhelp32.h>
#include <winhttp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include <stdarg.h>

#ifdef _MSC_VER
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#define W_RST  L"\x1b[0m"
#define W_CYN  L"\x1b[1;36m"
#define W_YEL  L"\x1b[1;33m"
#define W_GRY  L"\x1b[90m"
#define W_WHT  L"\x1b[1;37m"
#define W_GRN  L"\x1b[1;32m"
#define W_RED  L"\x1b[1;31m"
#define W_MAG  L"\x1b[1;35m"

#define CDP_HOST        L"localhost"
#define CDP_PORT        9222
#define INI_SECTION     L"Config"
#define INI_KEY_NOTIFY  L"Notify"
#define AVISO_MS        1200
#define FUENTE_ALTO     16
#define FILAS_EXTRA     3
#define COLS_EXTRA      2
#define LISTEN_MS       (30ULL * 60 * 1000)
#define BUFFER_FILAS    9999

static HANDLE hOut;
static int    g_notify = 1;
static wchar_t g_ini_path[MAX_PATH];

static const wchar_t *ART_LINES[] = {
    L"\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28f8\u2880\u2846\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28a7\u2800\u2800\u2800\u2808\u2811\u288c\u28e6\u2840\u2800\u28e0\u2804\u2801\u2800\u2808\u2806\u2800\u2800\u2800\u28b0\u2840\u2800\u28b9\u28ff\u28e6",
    L"\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2847\u285c\u28bb\u2840\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28b8\u28a7\u2800\u2800\u2800\u2800\u2800\u28f9\u28ff\u28ce\u2801\u2800\u2800\u2800\u2850\u2884\u2800\u2800\u2800\u2800\u28b7\u2844\u2808\u28ff\u28ff",
    L"\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28ff\u2801\u2808\u28c7\u2808\u2844\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2818\u2848\u28b6\u2886\u28e0\u28f4\u281e\u280b\u2800\u2819\u2837\u28c4\u2800\u2800\u2819\u28a6\u2840\u2840\u2800\u2800\u2838\u2859\u2840\u28ff\u28ff",
    L"\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u284f\u2800\u2800\u2838\u2844\u28b1\u2840\u2800\u2800\u2800\u2800\u2800\u2830\u2840\u2800\u2800\u2820\u28e4\u28f7\u283e\u283f\u28ff\u28c4\u28c0\u2800\u2800\u2800\u2800\u283a\u28b7\u28e6\u2840\u2800\u2819\u28a6\u2800\u2840\u2800\u2847\u2810\u28bf\u28ff",
    L"\u2800\u28b0\u2864\u28e4\u28c0\u28c0\u28c0\u28f7\u28c7\u2840\u2800\u2800\u2839\u2844\u28a7\u2800\u2800\u2800\u28c0\u2874\u2882\u2837\u2840\u2810\u28a4\u28e9\u28f1\u2800\u28a0\u281e\u283b\u28d5\u28fb\u28e6\u28f4\u28fe\u28ff\u28f6\u28fe\u287f\u283f\u28bf\u28ff\u2857\u2847\u2800\u2847\u2800\u2808\u283b",
    L"\u2800\u2800\u28a7\u2800\u2800\u2800\u2809\u2839\u284f\u2809\u2809\u2809\u2809\u281b\u28df\u28df\u2809\u2809\u2801\u2800\u280a\u2800\u2801\u2800\u2800\u2808\u28bf\u2840\u280b\u28a0\u28f6\u28ff\u28ff\u28ff\u28ff\u28ff\u28c9\u2800\u2800\u2800\u2800\u28b8\u2847\u28ff\u2807\u28b8\u2857\u2880\u2800\u2809",
    L"\u2800\u2800\u2818\u28f6\u28c4\u2800\u2800\u28a3\u2879\u2840\u2800\u2800\u2800\u2808\u2809\u282f\u28ff\u28e6\u2844\u2820\u2840\u2800\u2800\u2830\u28c0\u2800\u2818\u28c7\u2800\u28b9\u287f\u2809\u28ff\u28ff\u28ed\u284d\u28ff\u2846\u2800\u2800\u2800\u287f\u2800\u28ff\u2800\u28fc\u28b7\u28e6\u28c4\u28c0",
    L"\u2800\u2800\u2800\u28bb\u287f\u28e6\u28c4\u2800\u28b3\u28f7\u28e4\u28f4\u28f6\u28f6\u28f6\u28f6\u28fd\u28fb\u281b\u2826\u282c\u2837\u2826\u2824\u28c8\u28f3\u28c4\u2858\u28c6\u2808\u2800\u2800\u28bb\u28c6\u281b\u2881\u28ff\u2807\u2800\u2800\u2818\u2887\u287c\u2847\u28f0\u283f\u2800\u2838\u2809\u28bb",
    L"\u2846\u2800\u28a0\u2808\u28a7\u2888\u28e2\u28ff\u28fe\u283f\u283f\u289b\u28ff\u28ff\u28ff\u28ef\u2849\u2819\u2837\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2809\u2809\u2801\u2800\u2800\u2880\u28c8\u28c9\u282d\u2825\u2824\u2800\u2800\u2800\u2888\u287c\u281f\u28b9\u2800\u2800\u2800\u2800\u28b8",
    L"\u2808\u2857\u28bc\u28e7\u283c\u28ff\u284f\u2801\u2800\u2800\u2800\u28ff\u28df\u28b9\u28fe\u2819\u28f7\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28b8\u2800\u2800\u2800\u2800\u28fc",
    L"\u2800\u28bf\u2840\u28b9\u28a6\u28f1\u2858\u28a6\u2800\u2800\u2800\u2818\u28bf\u28c4\u28e5\u287e\u280b\u2880\u2840\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u281a\u2808",
    L"\u2800\u2800\u2879\u28fe\u2840\u2819\u28b7\u28c4\u2811\u2804\u2800\u2800\u2800\u2860\u2814\u2812\u280b\u2801\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800",
    L"\u28c3\u28fc\u28f7\u28ff\u281b\u28e6\u28c8\u28fb\u28dd\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28b0\u2800\u2800\u2800\u2800\u2880",
    L"\u28ff\u28ff\u2849\u28a0\u2846\u28a3\u2839\u284c\u2809\u2801\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2880\u28c0\u28a4\u28a4\u28c4\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28b8\u2846\u2800\u2800\u2800\u28b8",
    L"\u28ff\u285c\u28b7\u28c8\u283f\u284c\u2844\u28bb\u28c4\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2880\u28e0\u2824\u2894\u28ca\u28e5\u28f4\u287e\u283f\u283f\u28e7\u2840\u2800\u2800\u2800\u2800\u2800\u2800\u28fc\u2847\u2800\u2800\u2800\u28fc",
    L"\u287f\u2801\u2838\u281b\u28c6\u28f1\u2801\u2818\u285c\u28c6\u2800\u2800\u2800\u2810\u2840\u2800\u2800\u2800\u2800\u2800\u2800\u28a0\u28e4\u28d2\u28c9\u28ed\u28fd\u2876\u281e\u280b\u2809\u2801\u2800\u2800\u2800\u2800\u2838\u28b7\u2800\u2800\u2800\u2800\u2800\u28f0\u284f\u2847\u2800\u2800\u2800\u28bf",
    L"\u2847\u2800\u2800\u2810\u28ff\u2809\u285e\u2800\u28e7\u2858\u2886\u2800\u2800\u2800\u28a3\u2800\u2800\u2800\u2800\u2800\u2800\u28b8\u28ff\u281b\u2801\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28f8\u2800\u2800\u2800\u2800\u2874\u2801\u2847\u28c1\u2846\u2800\u2800\u28b8",
    L"\u28c7\u2857\u2800\u2800\u28ff\u2800\u28ff\u2800\u28b8\u2809\u283a\u28a7\u2840\u2800\u2800\u28f7\u2840\u2800\u2800\u2800\u2800\u2808\u28ff\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28c0\u2860\u2803\u2800\u2800\u28e0\u28fe\u2841\u2820\u28e7\u28fc\u2847\u2800\u2800\u2818",
    L"\u284f\u2800\u2800\u2800\u28ff\u28e4\u28ff\u28c7\u2838\u2846\u2800\u2800\u2831\u28c4\u2800\u2809\u2801\u2800\u2800\u2800\u2800\u2800\u2838\u2844\u2800\u2800\u2800\u28c0\u2860\u2824\u2812\u2812\u280b\u2809\u2801\u2800\u2800\u2800\u2880\u28fc\u28ff\u280f\u28a7\u2800\u28ff\u28ff\u2807\u2800\u2800\u2800",
    L"\u28e7\u2800\u2800\u2800\u28ff\u2807\u2847\u28b9\u2800\u2847\u2800\u2800\u2800\u2808\u2813\u2826\u28c0\u2800\u2800\u2800\u2800\u2800\u2800\u2808\u2812\u280a\u2809\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u28e0\u28f4\u28ff\u285f\u28b8\u2800\u2800\u2801\u28ff\u28ff\u2800\u2800\u2800\u2800",
    L"\u28ff\u2800\u2800\u2800\u28ff\u2800\u2847\u2808\u2847\u28a7\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2809\u2813\u28e6\u28e4\u28c0\u2840\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2880\u28e0\u28fe\u28ff\u28ff\u280f\u2800\u28b8\u28f7\u2844\u2800\u28b8\u28ff\u2800\u2800\u2800\u2800",
    L"\u28fb\u2800\u2800\u28a0\u28ff\u28b0\u2801\u28fc\u28c7\u28b8\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2820\u281e\u2801\u2808\u28ff\u283f\u28ff\u28f6\u28f6\u28e6\u28e4\u28e4\u28e4\u28c4\u28e0\u28f4\u28ff\u28ff\u28ff\u281f\u2801\u2800\u2800\u28f8\u287f\u28bf\u28c4\u2818\u2847\u2800\u2800\u2800\u2800",
    L"\u289f\u28e4\u281e\u2809\u284f\u287c\u2800\u28b8\u28bf\u28b8\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2880\u28f4\u28ff\u2840\u2800\u2809\u281b\u283f\u28bf\u28ff\u28ff\u28ff\u28ff\u287f\u281f\u280b\u2801\u2800\u2880\u2874\u281a\u2801\u2800\u2800\u28bf\u28ff\u2801\u2800\u2800\u2800\u2800",
    L"\u281f\u280b\u2800\u2800\u28ff\u2803\u2800\u2878\u2818\u28b8\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2880\u28f4\u28ff\u2803\u2808\u2813\u2824\u28c0\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2880\u2864\u281e\u2801\u2800\u2800\u2800\u2800\u2800\u2808\u284f\u2800\u2800\u2800\u2800\u2800",
    L"\u2800\u2800\u2800\u28a0\u281f\u2803\u28a0\u2807\u2800\u287c\u2800\u2800\u2800\u2800\u28c0\u28e4\u28f6\u28ff\u28ff\u2807\u2800\u2800\u2800\u2800\u2800\u2809\u2819\u2832\u2824\u28c4\u28c0\u28e4\u2854\u280a\u2801\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2800\u2878\u2801\u2800\u2800\u2840\u2800\u2800",
    NULL
};
#define ART_LINE_COUNT ((int)(sizeof(ART_LINES)/sizeof(ART_LINES[0]) - 1))

static const char *QUEST_SCRIPT_NORMAL =
    "(async function(){"
    "delete window.$;"
    "let r=webpackChunkdiscord_app.push([[Symbol()],{},x=>x]);"
    "webpackChunkdiscord_app.pop();"
    "function isWrapper(v){const k=Object.getOwnPropertyNames(v);return k.includes('$$baseObject')||k.includes('$$loader');}"
    "function getAllMethods(v){const m=new Set();let o=v;while(o&&o!==Object.prototype){for(const k of Object.getOwnPropertyNames(o)){try{if(typeof Object.getOwnPropertyDescriptor(o,k)?.value==='function')m.add(k);}catch(_){}}o=Object.getPrototypeOf(o);}return m;}"
    "function findStore(pred){for(const mod of Object.values(r.c)){if(!mod?.exports)continue;const exp=mod.exports;const cands=[exp,...Object.getOwnPropertyNames(exp).map(k=>{try{return exp[k];}catch(_){return null;}}).filter(Boolean)];for(const v of cands){if(!v||typeof v!=='object'||isWrapper(v)||!v._dispatchToken)continue;try{if(pred(getAllMethods(v)))return v;}catch(_){}}}return null;}"
    "function findPlain(pred){for(const mod of Object.values(r.c)){if(!mod?.exports)continue;const exp=mod.exports;const cands=[exp,...Object.getOwnPropertyNames(exp).map(k=>{try{return exp[k];}catch(_){return null;}}).filter(Boolean)];for(const v of cands){if(!v||isWrapper(v))continue;try{if(pred(v))return v;}catch(_){}}}return null;}"
    "const QuestsStore=findStore(m=>m.has('getQuest')&&m.has('getQuestConfig'));"
    "const RunningGameStore=findStore(m=>m.has('getRunningGames')&&m.has('getGameForPID')&&m.has('getVisibleRunningGames'));"
    "const AppStreamingStore=findStore(m=>m.has('getStreamerActiveStreamMetadata')&&m.has('getAllActiveStreams'));"
    "const ChannelStore=findStore(m=>m.has('getSortedPrivateChannels')&&m.has('getAllThreadsForParent'));"
    "const GuildChannelStore=findStore(m=>m.has('getSFWDefaultChannel')&&m.has('getAllGuilds'));"
    "const FluxDispatcher=findPlain(v=>v._actionHandlers&&typeof v.dispatch==='function'&&typeof v.subscribe==='function');"
    "const AuthStore=findStore(m=>m.has('getToken')&&m.has('isAuthenticated'));"
    "const _rawApi=(()=>{const seen=[];for(const mod of Object.values(r.c)){if(!mod?.exports)continue;const exp=mod.exports;const cands=[exp,...Object.getOwnPropertyNames(exp).map(k=>{try{return exp[k];}catch(_){return null;}}).filter(Boolean)];for(const v of cands){if(!v||isWrapper(v)||v._dispatchToken)continue;if(typeof v.get==='function'&&typeof v.post==='function'&&typeof v.patch==='function'&&typeof v.del==='function')seen.push(v);}}return seen[0];})();"
    "const token=AuthStore.getToken();"
    "const api={"
    "  get:(url)=>_rawApi.get(`/api/v9${url}`).set('Authorization',token),"
    "  post:(url,body)=>_rawApi.post(`/api/v9${url}`).set('Authorization',token).send(body),"
    "  patch:(url,body)=>_rawApi.patch(`/api/v9${url}`).set('Authorization',token).send(body),"
    "  del:(url)=>_rawApi.del(`/api/v9${url}`).set('Authorization',token),"
    "};"
    "if(!QuestsStore||!FluxDispatcher||!_rawApi||!token){console.log('[QUEST] stores missing');return;}"
    "const supportedTasks=['WATCH_VIDEO','PLAY_ON_DESKTOP','STREAM_ON_DESKTOP','PLAY_ACTIVITY','WATCH_VIDEO_ON_MOBILE'];"
    "const done=new Set();"
    "const isDone=x=>{const us=x.userStatus;if(!us)return false;if(us.completedAt||us.claimedAt||done.has(x.id))return true;const tc=x.config.taskConfig??x.config.taskConfigV2;const tn=supportedTasks.find(t=>tc.tasks[t]!=null);const tgt=tc.tasks[tn]?.target;const v=x.config.configVersion===1?(us.streamProgressSeconds??0):(us.progress?.[tn]?.value??0);return us.progress?.[tn]?.completedAt!=null||(tgt!=null&&v>=tgt);};"
    "const refreshDone=async()=>{try{const rs=await api.get('/quests/@me');for(const q of (rs.body?.quests??[])){const us=q.user_status;if(us&&(us.completed_at||us.claimed_at))done.add(q.id);}}catch(_){console.log('[dbg-quest] no se pudo consultar /quests/@me');}};"
    "const getQuests=()=>[...QuestsStore.quests.values()].filter(x=>x.userStatus?.enrolledAt&&supportedTasks.find(y=>Object.keys((x.config.taskConfig??x.config.taskConfigV2).tasks).includes(y))&&!isDone(x)&&new Date(x.config.expiresAt).getTime()>Date.now());"
    "let quests=[];"
    "const isApp=typeof DiscordNative!=='undefined';"
    "for(let i=0;i<30&&QuestsStore.quests.size===0;i++){console.log('[QUEST] esperando que Discord cargue las quests ('+(i+1)+'/30)');await new Promise(f=>setTimeout(f,2000));}"
    "await new Promise(f=>setTimeout(f,1500));"
    "await refreshDone();"
    "for(const x of QuestsStore.quests.values()){try{console.log('[dbg-quest] '+x.config.messages.questName+' enrolled='+!!x.userStatus?.enrolledAt+' completed='+!!x.userStatus?.completedAt+' claimed='+!!x.userStatus?.claimedAt+' done='+isDone(x));}catch(_){}}"
    "quests=getQuests();"
    "if(quests.length===0){console.log('[QUEST] no hay quests pendientes');return;}"
    "function doJob(){"
    "const quest=quests.pop();if(!quest)return;"
    "const pid=Math.floor(Math.random()*30000)+1000;"
    "const questName=quest.config.messages.questName;"
    "const taskConfig=quest.config.taskConfig??quest.config.taskConfigV2;"
    "const taskName=supportedTasks.find(x=>taskConfig.tasks[x]!=null);"
    "const taskData=taskConfig.tasks[taskName];"
    "const applicationId=quest.config.application?.id??taskData.applications?.[0]?.id;"
    "const secondsNeeded=taskData.target;"
    "let secondsDone=quest.userStatus?.progress?.[taskName]?.value??0;"
    "if(taskName==='WATCH_VIDEO'||taskName==='WATCH_VIDEO_ON_MOBILE'){"
    "const speed=7;let completed=false;"
    "(async()=>{while(true){const remaining=Math.min(speed,secondsNeeded-secondsDone);"
    "await new Promise(res=>setTimeout(res,remaining*1000));"
    "const timestamp=secondsDone+speed;"
    "const res=await api.post(`/quests/${quest.id}/video-progress`,{timestamp:Math.min(secondsNeeded,timestamp+Math.random())});"
    "completed=res.body.completed_at!=null;secondsDone=Math.min(secondsNeeded,timestamp);"
    "if(timestamp>=secondsNeeded)break;}"
    "if(!completed)await api.post(`/quests/${quest.id}/video-progress`,{timestamp:secondsNeeded});"
    "console.log('[QUEST] completed:'+questName);doJob();})();"
    "console.log('[QUEST] video:'+questName);"
    "}else if(taskName==='PLAY_ON_DESKTOP'){"
    "if(!isApp){console.log('[QUEST] needs desktop app');return;}"
    "api.get(`/applications/public?application_ids=${applicationId}`).then(res=>{"
    "const appData=res.body[0];"
    "const exeName=appData.executables?.find(x=>x.os==='win32')?.name?.replace('>','')||appData.name.replace(/[\\/\\\\:*?\"<>|]/g,'');"
    "const fakeGame={cmdLine:`C:\\\\Program Files\\\\${appData.name}\\\\${exeName}`,exeName,exePath:`c:/program files/${appData.name.toLowerCase()}/${exeName}`,hidden:false,isLauncher:false,id:applicationId,name:appData.name,pid,pidPath:[pid],processName:appData.name,start:Date.now()};"
    "const realGames=RunningGameStore.getRunningGames();"
    "const realGetRunningGames=RunningGameStore.getRunningGames;"
    "const realGetGameForPID=RunningGameStore.getGameForPID;"
    "const fakeGames=[fakeGame];"
    "RunningGameStore.getRunningGames=()=>fakeGames;"
    "RunningGameStore.getGameForPID=p=>fakeGames.find(x=>x.pid===p);"
    "FluxDispatcher.dispatch({type:'RUNNING_GAMES_CHANGE',removed:realGames,added:[fakeGame],games:fakeGames});"
    "const fn=data=>{"
    "const progress=quest.config.configVersion===1?data.userStatus.streamProgressSeconds:Math.floor(data.userStatus.progress.PLAY_ON_DESKTOP.value);"
    "console.log('[QUEST] progress:'+progress+'/'+secondsNeeded);"
    "if(progress>=secondsNeeded){console.log('[QUEST] completed:'+questName);"
    "RunningGameStore.getRunningGames=realGetRunningGames;RunningGameStore.getGameForPID=realGetGameForPID;"
    "FluxDispatcher.dispatch({type:'RUNNING_GAMES_CHANGE',removed:[fakeGame],added:[],games:[]});"
    "FluxDispatcher.unsubscribe('QUESTS_SEND_HEARTBEAT_SUCCESS',fn);doJob();}};"
    "FluxDispatcher.subscribe('QUESTS_SEND_HEARTBEAT_SUCCESS',fn);"
    "console.log('[QUEST] spoofed:'+appData.name+':'+Math.ceil((secondsNeeded-secondsDone)/60)+'min');});"
    "}else if(taskName==='STREAM_ON_DESKTOP'){"
    "if(!isApp){console.log('[QUEST] needs desktop app');return;}"
    "const realFunc=AppStreamingStore.getStreamerActiveStreamMetadata;"
    "AppStreamingStore.getStreamerActiveStreamMetadata=()=>({id:applicationId,pid,sourceName:null});"
    "const fn=data=>{"
    "const progress=quest.config.configVersion===1?data.userStatus.streamProgressSeconds:Math.floor(data.userStatus.progress.STREAM_ON_DESKTOP.value);"
    "console.log('[QUEST] progress:'+progress+'/'+secondsNeeded);"
    "if(progress>=secondsNeeded){console.log('[QUEST] completed:'+questName);"
    "AppStreamingStore.getStreamerActiveStreamMetadata=realFunc;"
    "FluxDispatcher.unsubscribe('QUESTS_SEND_HEARTBEAT_SUCCESS',fn);doJob();}};"
    "FluxDispatcher.subscribe('QUESTS_SEND_HEARTBEAT_SUCCESS',fn);"
    "console.log('[QUEST] stream spoofed:'+Math.ceil((secondsNeeded-secondsDone)/60)+'min');"
    "}else if(taskName==='PLAY_ACTIVITY'){"
    "const channelId=ChannelStore?.getSortedPrivateChannels()[0]?.id??Object.values(GuildChannelStore.getAllGuilds()).find(x=>x?.VOCAL?.length>0)?.VOCAL[0]?.channel?.id;"
    "if(!channelId){console.log('[QUEST] no channel');return;}"
    "const streamKey=`call:${channelId}:1`;"
    "(async()=>{console.log('[QUEST] activity:'+questName);"
    "while(true){"
    "const res=await api.post(`/quests/${quest.id}/heartbeat`,{stream_key:streamKey,terminal:false});"
    "const progress=res.body.progress.PLAY_ACTIVITY.value;"
    "console.log('[QUEST] progress:'+progress+'/'+secondsNeeded);"
    "await new Promise(res=>setTimeout(res,20*1000));"
    "if(progress>=secondsNeeded){await api.post(`/quests/${quest.id}/heartbeat`,{stream_key:streamKey,terminal:true});break;}}"
    "console.log('[QUEST] completed:'+questName);doJob();})();}"
    "}doJob();})();";

static void wprint(const wchar_t *s) {
    DWORD w;
    WriteConsoleW(hOut, s, (DWORD)wcslen(s), &w, NULL);
}

static void wprintf_c(const wchar_t *fmt, ...) {
    wchar_t buf[2048];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf(buf, 2047, fmt, ap);
    va_end(ap);
    buf[2047] = 0;
    wprint(buf);
}

static void console_init(void) {
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    CONSOLE_FONT_INFOEX cfi;
    memset(&cfi, 0, sizeof(cfi));
    cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    cfi.dwFontSize.Y = FUENTE_ALTO;
    cfi.FontWeight   = 400;
    cfi.FontFamily   = FF_DONTCARE;
    wcscpy(cfi.FaceName, L"NSimSun");
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    Sleep(80);
}

static void cls(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
        wprint(L"\x1b[2J\x1b[H");
        return;
    }
    DWORD n = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y, w;
    COORD origen = {0, 0};
    FillConsoleOutputCharacterW(hOut, L' ', n, origen, &w);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, n, origen, &w);
    SetConsoleCursorPosition(hOut, origen);
    SMALL_RECT win = {0, 0,
                      (SHORT)(csbi.srWindow.Right  - csbi.srWindow.Left),
                      (SHORT)(csbi.srWindow.Bottom - csbi.srWindow.Top)};
    SetConsoleWindowInfo(hOut, TRUE, &win);
}

static void ajustar_ventana(int cols, int filas) {
    static int last_c = 0, last_f = 0;
    if (cols == last_c && filas == last_f) return;
    last_c = cols; last_f = filas;
    COORD max = GetLargestConsoleWindowSize(hOut);
    int vc = cols, vf = filas;
    if (max.X > 0 && vc > max.X) vc = max.X;
    if (max.Y > 0 && vf > max.Y) vf = max.Y;
    SMALL_RECT minimo = {0, 0, 1, 1};
    SetConsoleWindowInfo(hOut, TRUE, &minimo);

    COORD buf = {(SHORT)cols, (SHORT)BUFFER_FILAS};
    SetConsoleScreenBufferSize(hOut, buf);
    SMALL_RECT win = {0, 0, (SHORT)(vc - 1), (SHORT)(vf - 1)};
    SetConsoleWindowInfo(hOut, TRUE, &win);
}

static void hsv2rgb(double h, int *r, int *g, int *b) {
    double s = 1.0, v = 1.0;
    double c = v * s;
    double hp = fmod(h, 360.0) / 60.0;
    double x = c * (1.0 - fabs(fmod(hp, 2.0) - 1.0));
    double r1 = 0, g1 = 0, b1 = 0;
    if      (hp < 1) { r1 = c; g1 = x; }
    else if (hp < 2) { r1 = x; g1 = c; }
    else if (hp < 3) { g1 = c; b1 = x; }
    else if (hp < 4) { g1 = x; b1 = c; }
    else if (hp < 5) { r1 = x; b1 = c; }
    else             { r1 = c; b1 = x; }
    double m = v - c;
    *r = (int)((r1 + m) * 255);
    *g = (int)((g1 + m) * 255);
    *b = (int)((b1 + m) * 255);
}

static void print_banner(void) {
    for (int y = 0; y < ART_LINE_COUNT; y++) {
        const wchar_t *line = ART_LINES[y];
        size_t len = wcslen(line);

        wchar_t *out = (wchar_t *)malloc((len * 32 + 32) * sizeof(wchar_t));
        if (!out) continue;
        size_t pos = 0;
        out[pos++] = L' ';
        out[pos++] = L' ';
        for (size_t x = 0; x < len; x++) {
            int r, g, b;
            hsv2rgb((double)(x * 6 + y * 18), &r, &g, &b);
            pos += _snwprintf(out + pos, 31, L"\x1b[38;2;%d;%d;%dm", r, g, b);
            out[pos++] = line[x];
        }
        wcscpy(out + pos, W_RST L"\n");
        wprint(out);
        free(out);
    }
}

#define MENU_WIDTH 56

static void print_separador(void) {
    wprint(L"  " W_CYN);
    for (int i = 0; i < MENU_WIDTH; i++) wprint(L"\u2500");
    wprint(W_RST L"\n");
}

static void print_menu(void) {
    wprint(L"\n");
    print_separador();
    wprintf_c(L"  " W_YEL L"[01]" W_RST L"  Iniciar              " W_GRY L"(mensajes simples)" W_RST L"\n\n");
    wprintf_c(L"  " W_YEL L"[02]" W_RST L"  Iniciar en debug     " W_GRY L"(mensajes completos)" W_RST L"\n\n");
    wprintf_c(L"  " W_YEL L"[03]" W_RST L"  Configuracion\n\n");
    print_separador();
    wprint(L"  " W_YEL L"[00]" W_RST L"  Salir\n\n");
}

static void pantalla(void) {

    int a_anc = 0;
    for (int i = 0; i < ART_LINE_COUNT; i++) {
        int len = (int)wcslen(ART_LINES[i]);
        if (len > a_anc) a_anc = len;
    }
    int cols  = ((a_anc + 4 > MENU_WIDTH + 4) ? a_anc + 4 : MENU_WIDTH + 4) + COLS_EXTRA;
    int filas = ART_LINE_COUNT + 12 + FILAS_EXTRA;
    ajustar_ventana(cols, filas);
    cls();
    print_banner();
    print_menu();
}

static void ini_path_init(void) {
    wchar_t appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
        _snwprintf(g_ini_path, MAX_PATH - 1, L"%ls\\discord_quest.ini", appdata);
    } else {
        wcscpy(g_ini_path, L"discord_quest.ini");
    }
}

static void config_load(void) {
    g_notify = (int)GetPrivateProfileIntW(INI_SECTION, INI_KEY_NOTIFY, 1, g_ini_path);
}

static void config_save(void) {
    wchar_t buf[8];
    _snwprintf(buf, 7, L"%d", g_notify);
    WritePrivateProfileStringW(INI_SECTION, INI_KEY_NOTIFY, buf, g_ini_path);
}

static void toast_register_aumid(void) {
    HKEY hk;
    if (RegCreateKeyExW(HKEY_CURRENT_USER,
                        L"Software\\Classes\\AppUserModelId\\DiscordQuest",
                        0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) {
        const wchar_t *name = L"Discord Quest";
        DWORD one = 1;
        RegSetValueExW(hk, L"DisplayName", 0, REG_SZ,
                       (const BYTE *)name, (DWORD)((wcslen(name) + 1) * sizeof(wchar_t)));
        RegSetValueExW(hk, L"ShowInSettings", 0, REG_DWORD, (const BYTE *)&one, sizeof(one));
        RegCloseKey(hk);
    }
}

static void tray_notify(const wchar_t *msg) {
    toast_register_aumid();
    SetEnvironmentVariableW(L"DQ_MSG", msg);

    wchar_t cmd[2048];
    wcscpy(cmd,
        L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass "
        L"-WindowStyle Hidden -Command \""
        L"[void][Windows.UI.Notifications.ToastNotificationManager,Windows.UI.Notifications,ContentType=WindowsRuntime];"
        L"$x=[Windows.UI.Notifications.ToastNotificationManager]::GetTemplateContent([Windows.UI.Notifications.ToastTemplateType]::ToastText02);"
        L"$t=$x.GetElementsByTagName('text');"
        L"[void]$t.Item(0).AppendChild($x.CreateTextNode('Discord Quest'));"
        L"[void]$t.Item(1).AppendChild($x.CreateTextNode($env:DQ_MSG));"
        L"$n=New-Object -TypeName Windows.UI.Notifications.ToastNotification -ArgumentList $x;"
        L"[Windows.UI.Notifications.ToastNotificationManager]::CreateToastNotifier('DiscordQuest').Show($n)"
        L"\"");

    STARTUPINFOW si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    memset(&pi, 0, sizeof(pi));
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

static char *cdp_http_get(const wchar_t *path) {
    HINTERNET hSession = WinHttpOpen(L"discord_quest/1.0",
                                     WINHTTP_ACCESS_TYPE_NO_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return NULL;

    HINTERNET hConn = WinHttpConnect(hSession, CDP_HOST, CDP_PORT, 0);
    if (!hConn) { WinHttpCloseHandle(hSession); return NULL; }

    HINTERNET hReq = WinHttpOpenRequest(hConn, L"GET", path, NULL,
                                         WINHTTP_NO_REFERER,
                                         WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hReq) { WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession); return NULL; }

    if (!WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                             WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(hReq, NULL)) {
        WinHttpCloseHandle(hReq);
        WinHttpCloseHandle(hConn);
        WinHttpCloseHandle(hSession);
        return NULL;
    }

    char   *buf  = NULL;
    size_t  total = 0;
    DWORD   avail = 0;
    while (WinHttpQueryDataAvailable(hReq, &avail) && avail > 0) {
        char  *tmp  = (char *)realloc(buf, total + avail + 1);
        DWORD  read = 0;
        if (!tmp) break;
        buf = tmp;
        WinHttpReadData(hReq, buf + total, avail, &read);
        total += read;
        buf[total] = '\0';
    }

    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConn);
    WinHttpCloseHandle(hSession);
    return buf;
}

static int cdp_evaluate(const wchar_t *ws_url_path, const char *expression,
                         int debug_mode,
                         char **out_result) {
    *out_result = NULL;

    HINTERNET hSession = WinHttpOpen(L"discord_quest/1.0",
                                     WINHTTP_ACCESS_TYPE_NO_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS,
                                     0);
    if (!hSession) return 0;

    HINTERNET hConn = WinHttpConnect(hSession, CDP_HOST, CDP_PORT, 0);
    if (!hConn) { WinHttpCloseHandle(hSession); return 0; }

    HINTERNET hReq = WinHttpOpenRequest(hConn, L"GET", ws_url_path, NULL,
                                         WINHTTP_NO_REFERER,
                                         WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hReq) { WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession); return 0; }

    WinHttpSetOption(hReq, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0);

    if (!WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                             WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(hReq, NULL)) {
        WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession);
        return 0;
    }

    HINTERNET hWs = WinHttpWebSocketCompleteUpgrade(hReq, 0);
    WinHttpCloseHandle(hReq);
    if (!hWs) { WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession); return 0; }

    size_t expr_len = strlen(expression);
    char  *escaped  = (char *)malloc(expr_len * 2 + 1);
    if (!escaped) goto cleanup;
    {
        size_t j = 0;
        for (size_t i = 0; i < expr_len; i++) {
            char c = expression[i];
            if      (c == '\\') { escaped[j++] = '\\'; escaped[j++] = '\\'; }
            else if (c == '"')  { escaped[j++] = '\\'; escaped[j++] = '"';  }
            else if (c == '\n') { escaped[j++] = '\\'; escaped[j++] = 'n';  }
            else if (c == '\r') { escaped[j++] = '\\'; escaped[j++] = 'r';  }
            else                { escaped[j++] = c; }
        }
        escaped[j] = '\0';
    }

    char msg[65536];
    int  msg_len = _snprintf(msg, sizeof(msg) - 1,
        "{\"id\":1,\"method\":\"Runtime.evaluate\","
        "\"params\":{\"expression\":\"%s\","
        "\"awaitPromise\":false,\"returnByValue\":true}}",
        escaped);
    free(escaped);
    if (msg_len <= 0) goto cleanup;

    const char *enable_msg = "{\"id\":0,\"method\":\"Runtime.enable\"}";
    if (WinHttpWebSocketSend(hWs, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
                             (PVOID)enable_msg, (DWORD)strlen(enable_msg)) != ERROR_SUCCESS)
        goto cleanup;

    if (WinHttpWebSocketSend(hWs, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
                             (PVOID)msg, (DWORD)msg_len) != ERROR_SUCCESS)
        goto cleanup;

    char   *rbuf   = (char *)malloc(1 << 20);
    if (!rbuf) goto cleanup;

    ULONGLONG t0 = GetTickCount64();
    int     got_result = 0;

    while (GetTickCount64() - t0 < LISTEN_MS) {
        DWORD          bytes_read = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE buf_type;
        DWORD rc = WinHttpWebSocketReceive(hWs, rbuf, (1 << 20) - 1,
                                            &bytes_read, &buf_type);
        if (rc != ERROR_SUCCESS) break;
        rbuf[bytes_read] = '\0';

        if (strstr(rbuf, "Runtime.consoleAPICalled")) {
            char *arg = strstr(rbuf, "\"value\":\"");
            if (arg) {
                arg += 9;
                char *end = strchr(arg, '"');
                if (end) {
                    *end = '\0';

                    wchar_t wbuf[1024];
                    MultiByteToWideChar(CP_UTF8, 0, arg, -1, wbuf, 1023);
                    wbuf[1023] = 0;

                    if (debug_mode) {
                        wprintf_c(W_GRY L"  [dbg] %ls" W_RST L"\n", wbuf);
                    } else {

                        if (wcsncmp(wbuf, L"[QUEST]", 7) == 0) {
                            wprintf_c(W_GRN L"  > %ls" W_RST L"\n", wbuf + 8);
                        }
                    }

                    if (wcsncmp(wbuf, L"[QUEST]", 7) == 0 && wcsstr(wbuf, L"completed")) {
                        if (g_notify) {
                            const wchar_t *nm = wcsstr(wbuf, L"completed:");
                            wchar_t m[300];
                            _snwprintf(m, 299, L"Quest completada: %ls", nm ? nm + 10 : L"");
                            m[299] = 0;
                            tray_notify(m);
                        }
                    }

                    *end = '"';
                }
            }
        }

        if (!got_result && strncmp(rbuf, "{\"id\":1,", 6) == 0) {
            got_result = 1;
            if (*out_result == NULL) {
                *out_result = _strdup(rbuf);
            }
        }

    }
    free(rbuf);

cleanup:
    WinHttpWebSocketClose(hWs, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
    WinHttpCloseHandle(hWs);
    WinHttpCloseHandle(hConn);
    WinHttpCloseHandle(hSession);
    return got_result;
}

static int find_discord_exe(wchar_t *out, int out_len) {
    wchar_t localapp[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localapp)))
        return 0;

    wchar_t pattern[MAX_PATH];
    _snwprintf(pattern, MAX_PATH - 1, L"%ls\\Discord\\app-*", localapp);

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    wchar_t best[MAX_PATH] = {0};
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;

        if (wcscmp(fd.cFileName, best) > 0)
            wcscpy(best, fd.cFileName);
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);

    if (!best[0]) return 0;

    _snwprintf(out, out_len - 1, L"%ls\\Discord\\%ls\\Discord.exe", localapp, best);
    return GetFileAttributesW(out) != INVALID_FILE_ATTRIBUTES;
}

static int relaunch_discord(void) {
    wchar_t discord_exe[MAX_PATH];

    if (!find_discord_exe(discord_exe, MAX_PATH)) {
        wprint(W_RED L"  No se encontro Discord.exe en %LocalAppData%\\Discord\\app-*\n" W_RST);
        return 0;
    }

    wprintf_c(W_GRY L"  Usando: %ls\n" W_RST, discord_exe);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"Discord.exe") == 0) {
                    HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hp) { TerminateProcess(hp, 0); CloseHandle(hp); }
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    Sleep(2000);

    wchar_t cmd[MAX_PATH + 64];
    _snwprintf(cmd, MAX_PATH + 63,
               L"\"%ls\" --remote-debugging-port=%d",
               discord_exe, CDP_PORT);

    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hNul = CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
                              OPEN_EXISTING, 0, NULL);

    STARTUPINFOW si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));
    if (hNul != INVALID_HANDLE_VALUE) {
        si.dwFlags    = STARTF_USESTDHANDLES;
        si.hStdInput  = hNul;
        si.hStdOutput = hNul;
        si.hStdError  = hNul;
    }
    BOOL okp = CreateProcessW(NULL, cmd, NULL, NULL,
                              hNul != INVALID_HANDLE_VALUE,
                              DETACHED_PROCESS, NULL, NULL, &si, &pi);
    if (hNul != INVALID_HANDLE_VALUE) CloseHandle(hNul);
    if (!okp) return 0;
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);

    wprint(W_YEL L"  Discord relanzado. Esperando que cargue..." W_RST L"\n");

    for (int i = 0; i < 20; i++) {
        Sleep(1000);
        char *probe = cdp_http_get(L"/json/version");
        if (probe) { free(probe); return 1; }
    }
    return 0;
}

static wchar_t *find_discord_target(void) {
    char *json = cdp_http_get(L"/json");
    if (!json) return NULL;

    wchar_t *ws_path = NULL;
    char    *p       = json;

    while ((p = strstr(p, "\"webSocketDebuggerUrl\"")) != NULL) {
        char *url_start = strchr(p, ':');
        if (!url_start) break;
        url_start++;
        while (*url_start == ' ') url_start++;
        if (*url_start == '"') url_start++;

        char *url_end = strchr(url_start, '"');
        if (!url_end) break;
        *url_end = '\0';

        char *path_start = strstr(url_start, "/devtools");
        if (path_start) {

            char *block_start = p;
            while (block_start > json && *block_start != '{') block_start--;

            if (strstr(block_start, "\"page\"") || strstr(block_start, "\"iframe\"")) {

                int is_discord = (strstr(block_start, "discord") != NULL);

                if (is_discord || ws_path == NULL) {
                    int n = MultiByteToWideChar(CP_UTF8, 0, path_start, -1, NULL, 0);
                    wchar_t *tmp = (wchar_t *)malloc((n + 1) * sizeof(wchar_t));
                    if (tmp) {
                        MultiByteToWideChar(CP_UTF8, 0, path_start, -1, tmp, n);
                        free(ws_path);
                        ws_path = tmp;
                        if (is_discord) break;
                    }
                }
            }
            *url_end = '"';
        }
        p = url_end + 1;
    }

    free(json);
    return ws_path;
}

static void pantalla_config(void) {
    for (;;) {
        cls();
        print_banner();
        wprint(L"\n");
        print_separador();
        wprintf_c(L"  " W_CYN L"Configuracion" W_RST L"\n\n");
        wprintf_c(L"  " W_YEL L"[01]" W_RST L"  Notificacion al completar: %ls\n\n",
                  g_notify ? (W_GRN L"ACTIVADA" W_RST) : (W_RED L"DESACTIVADA" W_RST));
        print_separador();
        wprint(L"  " W_YEL L"[00]" W_RST L"  Volver\n\n");
        print_separador();

        wprint(W_WHT L"  Selecciona > " W_RST);
        char buf[16];
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int op = -1;
        sscanf(buf, "%d", &op);

        if (op == 0) break;
        if (op == 1) {
            g_notify ^= 1;
            config_save();
            wprintf_c(L"  " W_YEL L"Notificacion %ls." W_RST L"\n",
                      g_notify ? L"activada" : L"desactivada");
            Sleep(AVISO_MS);
        }
    }
}

static void run_injection(int debug_mode) {
    cls();
    print_banner();
    wprint(L"\n");
    print_separador();
    wprintf_c(L"  " W_CYN L"Iniciando inyeccion%ls" W_RST L"\n\n",
              debug_mode ? L" [modo debug]" : L"");

    char *probe = cdp_http_get(L"/json/version");
    if (!probe) {
        wprint(W_YEL L"  Puerto CDP no disponible. Relanzando Discord...\n" W_RST);
        if (!relaunch_discord()) {
            wprint(W_RED L"  No se pudo relanzar Discord automaticamente.\n" W_RST);
            wprintf_c(L"  Agrega manualmente " W_YEL L"--remote-debugging-port=%d" W_RST
                      L" al shortcut de Discord y vuelve a intentar.\n", CDP_PORT);
            wprint(L"\n  Presiona Enter para volver...\n");
            getchar(); return;
        }
        probe = cdp_http_get(L"/json/version");
        if (!probe) {
            wprint(W_RED L"  Sigue sin responder el puerto CDP.\n" W_RST);
            wprint(L"\n  Presiona Enter para volver...\n");
            getchar(); return;
        }
    }
    if (debug_mode) wprintf_c(W_GRY L"  [dbg] CDP version: %hs\n" W_RST, probe);
    free(probe);

    wprint(W_GRY L"  Buscando target renderer de Discord...\n" W_RST);
    wchar_t *ws_path = find_discord_target();
    if (!ws_path) {
        wprint(W_RED L"  No se encontro target de Discord en el CDP.\n" W_RST);
        wprint(L"  Asegurate de que Discord este abierto y cargado.\n");
        wprint(L"\n  Presiona Enter para volver...\n");
        getchar(); return;
    }
    if (debug_mode) wprintf_c(W_GRY L"  [dbg] target: %ls\n" W_RST, ws_path);

    wprint(W_YEL L"  Inyectando script...\n" W_RST);
    print_separador();
    wprint(L"\n");

    char *result = NULL;
    int  ok      = cdp_evaluate(ws_path, QUEST_SCRIPT_NORMAL, debug_mode, &result);
    free(ws_path);

    if (debug_mode && result) {
        wprintf_c(W_GRY L"\n  [dbg] evaluate response: %hs\n" W_RST, result);
    }
    free(result);

    if (!ok) {
        wprint(W_RED L"\n  Error en la inyeccion via WebSocket CDP.\n" W_RST);
    } else {
        wprint(W_GRN L"\n  Script finalizado.\n" W_RST);
    }

    wprint(L"\n  Presiona Enter para volver al menu...\n");
    getchar();
}

int main(void) {
    console_init();
    ini_path_init();
    config_load();

    for (;;) {
        pantalla();
        wprint(W_WHT L"  Selecciona una opcion > " W_RST);

        char buf[32];
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int op = -1;
        if (sscanf(buf, "%d", &op) != 1) op = -1;

        if (op == 0) break;

        if (op == 1) {
            run_injection(0);
        } else if (op == 2) {
            run_injection(1);
        } else if (op == 3) {
            pantalla_config();
        } else {
            wprint(L"  " W_YEL L"Opcion invalida." W_RST);
            Sleep(AVISO_MS);
        }
    }

    cls();
    wprint(L"  Hasta luego.\n");
    return 0;
}
