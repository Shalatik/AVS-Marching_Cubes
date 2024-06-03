Architektury Výpočetních Systémů (AVS 2023)
Projekt č. 2 (PMC)
Login: xcesko00

Úloha 1: Paralelizace původního řešení
===============================================================================

1) Kterou ze smyček (viz zadání) je vhodnější paralelizovat a co způsobuje 
   neefektivitu paralelizaci té druhé?

   Smyčka v metodě marchCubes() je nejvhodnější na paralelizaci, protože výpočet bude rozdělený mezi jádra efektivním způsobem.
   Paralelizace smyčky evaluateFieldAt() může zapříčinit zpomalení, protože může být neefektivní kvůli vysoké režii s vytvářením a správou vláken.
   
2) Jaké plánování (rozdělení práce mezi vlákna) jste zvolili a proč? 
   Jaký vliv má velikost "chunk" při dynamickém plánování (8, 16, 32, 64)?

   Při testování mi přišlo, že dynamic vykazuje konzistentně nejlepší výsledky.
   Co se týče vlivu velikosti "chunk", zde jsem si nevšimla žádného výrazného rozdílu a nechala jsem ho s velikostí 8.
   Velikost chunku určuje, kolik úloh se při dynamickém plánování přidělí jednomu vláknu.
   Při větším chunk size dojde k většímu zatížení vláken, což může způsobit zpomalení programu.

3) Jakým způsobem zajišťujete ukládání trojúhelníků z několika vláken současně?

   Pomocí OpenMP critical sekce (#pragma omp critical) při ukládání trojúhelníků do vektoru (mTriangles.push_back(triangle)).
   Kritická sekce zde zajistí, že v jednu chvíli bude přistupovat pouze jedno vlákno. Tohoto je nutné docílit pro udržení konzistence dat.
   


Úloha 2: Paralelní průchod stromem
===============================================================================

1) Stručně popište použití OpenMP tasků ve vašem řešení.
   
   Jako první se volá funkce createCube, toto se provede v metodě marchCubes().
   Zde se využívá #pragma omp parallel a #pragma omp single, které dohromadyzajistí, že se funkce na nejvyšší úrovni volá pouze jedním vláknem
   a zároveň se v createCube vytvoří tasky, které se následně vykonávají paralelně.

   V metodě createCube() se vytvoří 8 krychlí za použití jediného for cyklu. Pro každou krychli je vytvořen task, ve kterém se 
   rekurzivně volá metoda createCube(), dokud se nedostaneme na nejnižší úroveň. K tomuto se používá #pragma omp task shared(totalTriangles).
   Proměnná je shared, což umožní, aby se k ní dalo přistupovat z více vláken.
   #pragma omp atomic pro totalTriangles +=, aby se dala zapisovaly výsledky z vláken postupně.
   Jako poslední je zde využita #pragma omp taskwait, která zajistí, že se všechny tasky dokončí, než se pokračuje dále.
   Tímto předejdeme ztrátě dat.

2) Jaký vliv má na vaše řešení tzv. "cut-off"? Je vhodné vytvářet nový 
   task pro každou krychli na nejnižší úrovni?

   "Cut-off" zabezpečí, že jsme schopni využít paralelizaci na určitý počet vláken. Pomáhá v urychlení výpočtu a zvyšuje efektivitu algoritmu, protože se nebudou provádět zbytené výpočty.
   Dá se nastavit pomocí "cut-off" hodnoty. Není vhodné vytvářed nový task na nejnižší úrovni, jelikož by vznikalo příliš mnoho tasků, které by způsobily zpomalení programu.

3) Jakým způsobem zajišťujete ukládání trojúhelníků z několika vláken současně?

   Podobně jako v první úloze, zde se využívá #pragma omp critical, která zajistí, že bude v jednu chvíli přistupovat pouze jedno vlákno.
   Tato pragma označí úsek programu za kritickou sekci a při přístupu jednoho vlákna se zachovává konzistence programu a správnost výsledku.


Úloha 3: Grafy škálování všech řešení
===============================================================================

1) Stručně zhodnoťte efektivitu vytvořených řešení (na základě grafů ŠKÁLOVÁNÍ).

   Při porovnání časů Octree a OpenMP Loop je vidět, že Octree je obecně rychlejší(pro grid size 8 a 16 je velikost až moc malá na reálné porovnání výsledků, které má výpovědní hodnotu).
   Pouze v grafu grid_scaling je při velmi malé velikosti mřížky rychlost obou algoritmů srovnatelná.
   Octree implementace je rychlejší v s porovnání s Loop v závislosti na velikosti grid size v ms:
   grid_scaling_out.csv:

   8: 2(tree) 2(loop)
   16: 3(tree) 2(loop)
   32: 9(tree) 13(loop)
   64: 39(tree) 88(loop)
   128: 238(tree) 670(loop)
   256: 1672(tree) 5317(loop)
   512: 12635(tree) 42553(loop)

2) V jakém případě (v závislosti na počtu bodů ve vstupním souboru a velikosti 
   mřížky) bude vaše řešení 1. úlohy neefektivní? (pokud takový případ existuje)

   Z grafu input_scaling_weak vidíme, že pokud máme na vstupu malé množství bodů na vlákno a používáme hodně vláken, algoritmus začne být neefektivní.
   To se děje, protože provede každý výpočet, i ten který by se provést nemusel. Narozdíl od Octree algoritmu, kde se redukuje počet takových výpočtů.
   Jde o zrychlení referečního řešení, které není optimalizované pro takové případy.

3) Je (nebo není) stromový algoritmus efektivnější z pohledu slabého škálování 
   vzhledem ke vstupu?

   Octree je efektivnější z pohledu slabého škálování. Z grafu input_scaling_weak je vidět, že až na případ, kdy máme na vstupu velmi málo bodů na vlákno a vysoký
   počet vláken, je Octree rychlejší.

4) Jaký je rozdíl mezi silným a slabým škálováním?

   Silné škálování (Amdahlovo) je založeno na konstantním počtu dat a zvyšujícím se počtu zdrojů. Znázorňuje, jak se vyvíjí doba výpočtu v čase.
   V grafu input_scaling_weak vidíme, že při zvyšujícím se počtu vláken se prvně zrychluje výpočet, ale při velkém počtu vláken se výpočet začne zpomalovat.

   Slabé škálování (Gustafsonovo) je založeno na fixním čase, ve kterém se snažíme provést co nejvíce práce. 
   Zde bychom chtěli docílit toho, aby se výpočet zrychloval s rostoucím počtem vláken. Zároveň by se v ideálním případě
   měl rovnat čas výpočtu pro 1 jádro se vstupem 10 a výpočtu pro 2 jádra se vstupem 20. Zde tomu tak ale není.


Úloha 4: Analýza využití jader pomocí VTune
================================================================================

1) Jaké bylo průměrné využití jader pro všechny tři implementace s omezením na 
   18 vláken? Na kolik procent byly využity?

   | Implementace | Procentuální využití  | Průměrné využití jader |
   | ------------ | --------------------- | -------------------    |
   | ref          | 2.8%                  | 0.997                  |
   | loop         | 48.2%                 | 17.364                 |
   | tree         | 43.6%                 | 15.703                 |
   | ------------ | --------------------- | -------------------    |

2) Jaké bylo průměrné využití jader pro všechny tři implementace s využitím 
   všech jader? Na kolik procent se podařilo využít obě CPU?

   | Implementace | Procentuální využití  | Průměrné využití jader |
   | ------------ | --------------------- | -------------------    |
   | ref          | 2.8%                  | 0.997                  |
   | loop         | 78.0%                 | 28.089                 |
   | tree         | 75.3%                 | 27.124                 |
   | ------------ | --------------------- | -------------------    |

3) Jaké jsou závěry z těchto měření?

   Z těchto měření vidíme, že implementace Octree má průměně menší využití jader, než loop. 
   Toto je z důvodu, že se zde redukuje volání funkce buildCube, což snižuje nároky na výpočet.
   Neprovádí se zde tedy tolik výpočtů, což způsobí menší využití jader. Výpočet je ale rychlejší, než u loop.
   Díky použití "cut-off" metody a rozdělení na 8 krychlí se výpočet zrychlí.