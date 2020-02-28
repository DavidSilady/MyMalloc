# myMalloc

Zadanie:

V štandardnej knižnici jazyka C sú pre alokáciu a uvoľnenie pamäti k dispozícii funkcie
malloc, a free. V tomto zadaní je úlohou implementovať vlastnú verziu alokácie pamäti.

V prvej časti je vašou úlohou je implementovať nasledovné ŠTYRI funkcie využitím metódy
explicitných zoznamov alebo pokročilejšou:


    void *memory_alloc(unsigned int size);
    
    int memory_free(void *valid_ptr);
    
    int memory_check(void *ptr);
    
    void memory_init(void *ptr, unsigned int size);
    

Vo vlastnej implementácii môžete definovať aj iné pomocné funkcie ako vyššie spomenuté,
nesmiete však použiť existujúce funkcie malloc a free.

Funkcia memory_alloc má poskytovať služby analogické štandardnému malloc. Teda,
vstupné parametre sú veľkosť požadovaného súvislého bloku pamäte a funkcia mu vráti:
ukazovateľ na úspešne alokovaný kus voľnej pamäte, ktorý sa vyhradil, alebo NULL, keď nie
je možné súvislú pamäť požadovanej veľkosť vyhradiť.

Funkcia memory_free slúži na uvoľnenie vyhradeného bloku pamäti, podobne ako funkcia
free. Funkcia vráti 0, ak sa podarilo (funkcia zbehla úspešne) uvoľniť blok pamäti, inak vráti
1. Môžete predpokladať, že parameter bude vždy platný smerník z predchádzajúcich volaní
vrátení funkciou memory_alloc, ktorý ešte nebol uvoľnený.

Funkcia memory_check slúži na skontrolovanie, či parameter (smerník) je platný smerník,
ktorý bol v nejakom z predchádzajúcich volaní vrátení funkciou memory_alloc a zatiaľ
nebol uvoľnený funkciou memory_free. Funkcia vráti 0, ak sa je smerník neplatný, inak
vráti 1.

Funkcia memory_init slúži na inicializáciu spravovanej voľnej pamäte. Predpokladajte, že
funkcia sa volá práve raz pred všetkými inými volaniami memory_alloc, memory_free a
memory_check. Ako vstupný parameter funkcie príde blok pamäte,
ktorú môžete použiť pre organizovanie a aj pridelenie voľnej pamäte. 

Vaše funkcie nemôžu používať globálne premenné okrem jednej globálnej premennej na zapamätanie smerníku na
pamäť, ktorá vstupuje do funkcie memory_init. Smerníky, ktoré prideľuje vaša funkcia
memory_alloc musia byť výhradne z bloku pamäte, ktorá bola pridelená funkcii
memory_init.
