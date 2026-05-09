🔧 Tool folosit
Am folosit ChatGPT + Gemini pentru a genera și înțelege funcțiile:

parse_condition
match_condition
filter
🧠 Prompt-uri folosite
Pentru parse_condition/match_condition am folosit un prompt de tip:
I-am aratat documentul aferent cu cerinta cat si codul pe care l-am scris pana in acel punct. 
Pe parcurs tot i-am dat indicatii ca sa ajung la forma dorita.

⚙️ Cod generat de AI
AI-ul a generat:

folosirea strtok pentru parsare
comparații pentru:
int (severity)
string (category, inspector)
long (timestamp)
❌ Probleme întâlnite
1. 🔴 Modificarea string-ului original (bug clasic cu strtok)

Inițial, AI-ul aplica strtok direct pe input.

👉 Problemă:

strtok modifică string-ul original
dacă refoloseam string-ul → comportament imprevizibil

👉 Soluția mea:

char temp[128];
strncpy(temp, input, sizeof(temp) - 1);
temp[sizeof(temp) - 1] = '\0';

Am lucrat pe o copie (temp), nu pe input.

2. 🔴 Lipsa validării complete

AI-ul nu trata corect cazurile când:

lipsesc token-uri
format invalid (severity>=2 în loc de severity:>=:2) - nu conta asta asa de mult
- am avut problema in cazul in care aveam ceva de genul:
./city_manager --role inspector --user bob --filter downtown "severity:>=:2" category:==:road
nu conta category:==:road -> lua si pe acelea care aveau altceva, de ex: lightning 

i-am explicat problema si pana la urma a rezolvat

3. 🔴 Conversii de tip neclare

AI-ul nu era foarte explicit la:

atoi pentru int
atol pentru timestamp

👉 Am clarificat manual:

int val = atoi(value);
long val = atol(value);
4. 🔴 Lipsa unor operatori sau acoperire incompletă

Inițial, unele comparații nu erau complete (<=, >= etc.)

👉 Am completat toate cazurile:

if (strcmp(op, ">=") == 0) ...
5. 🔴 Lipsa extensibilității

AI-ul genera cod „hardcodat”.

👉 Am organizat codul pe secțiuni:

// --- SEVERITY ---
// --- CATEGORY ---
// --- INSPECTOR ---
// --- TIMESTAMP ---

pentru claritate și debug mai ușor.
PHASE 2
Instrument utilizat: Gemini

Cum m-a ajutat:
AI-ul a oferit structura pentru citirea PID-ului din fișierul .monitor_pid folosind open()/read() și a explicat cum să folosesc valoarea returnată de kill() pentru a verifica dacă semnalul a ajuns la destinație.

Probleme și Soluții:

Problema (Stale PID): Fișierul .monitor_pid putea exista chiar dacă monitorul era închis forțat.

Soluție: Am condiționat mesajul din log de rezultatul kill(pid, SIGUSR1). Dacă returnează -1, log-ul va raporta explicit că notificarea a eșuat.

Problema (Mesaje de log dinamice): Cerința cerea un mesaj clar pentru eșec.

Soluție: Am ajustat funcția add să pregătească un string diferit pentru log_action bazat pe variabila de stare notified.