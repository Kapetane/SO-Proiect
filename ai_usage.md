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