# MatchingEngine C++

**Moteur de matching d’ordres**  
Ce moteur prend en charge :

- **Types d’ordres**
    - **Limit Orders** : exécution à un prix défini (ou meilleur), insertion du reliquat dans le carnet
    - **Market Orders** : exécution immédiate au meilleur prix disponible

- **Actions supportées**
    - **NEW** : création d’un nouvel ordre
    - **MODIFY** : ajustement de la quantité d’un ordre existant (recalcul FIFO)
    - **CANCEL** : suppression d’un ordre en attente

- **Priorité**
    - **Prix-Temps** :
        - Carnet **achat** trié par prix décroissant puis ancienneté
        - Carnet **vente** trié par prix croissant puis ancienneté

- **Multi-instrument**
    - Un carnet dédié par instrument financier (nommé `"SYM0"`, `"AAPL"`, …)

- **Complexité**
    - **O(log M)** par insertion/match (M = nombre de niveaux de prix actifs)
    - Latence minimale grâce à l’usage de structures STL optimisées et d’un code C++17 pur


> Ce composant est conçu pour être **léger**, **portable** (Windows / Linux / macOS) et **hautement performant** pour les environnements de trading à faible latence.

---

## I - Vue d’ensemble

Ce projet fournit une chaîne de traitement complète pour un flux d’ordres financiers :

1. **Lecture** d’un fichier CSV d’ordres (`CsvParser`)
2. **Modélisation** de chaque ordre (`Order`)
3. **Matching** FIFO par prix et instrument (`MatchingEngine` + `OrderBook`)
4. **Production** de résultats de matching (`MatchResult`)
5. **Écriture** du flux de sorties au format CSV (`CsvWriter`)
6. **Tracing** de chaque événement métier via un logger simple (`Logger`)
7. **Tests unitaires** couvrant tous les modules (GoogleTest)
8. **Bench de performance** standalone pour mesurer le débit (`bench/Performance.cpp`)

---

## II - Architecture & Modules

Projet/
├─ bench/
│ └─ Performance.cpp # bench standalone
├─ data/
│ ├─ input.csv # exemple d’entrée
│ └─ output.csv # exemple de sortie
├─ include/ # headers publics
│ ├─ CsvParser.h
│ ├─ CsvWriter.h
│ ├─ Logger.h
│ ├─ MatchingEngine.h
│ ├─ MatchResult.h
│ ├─ Order.h
│ └─ OrderBook.h
├─ src/ # implémentations
│ ├─ CsvParser.cpp
│ ├─ CsvWriter.cpp
│ ├─ Logger.cpp
│ ├─ MatchingEngine.cpp
│ ├─ Order.cpp
│ └─ OrderBook.cpp
├─ tests/
│ ├─ data/ # CSV pour tests unitaires
│ └─ unit/
│ ├─ test_CsvParser.cpp
│ ├─ test_CsvWriter.cpp
│ ├─ test_MatchingEngine.cpp
│ ├─ test_OrderBook.cpp
│ └─ test_Performance.cpp
├─ CMakeLists.txt # build core, app, bench & tests
├─ README.md # cette documentation
└─ main.cpp # exécutable principal


---

## III - Détail des composants

### CsvParser
- **But** : lire un CSV d’ordres, sauter l’en-tête, découper chaque ligne, valider tous les champs.
- **Erreurs gérées** : mauvais nombre de colonnes, timestamp/order_id non numériques, instrument vide, side/type/action invalides, quantité/prix négatifs ou mal formés.
- **Usage** :
  ```cpp
  me::CsvParser parser("data/input.csv");
  while (auto maybe = parser.next()) {
    Order o = *maybe;
    // …
  }
  for (auto const& err : parser.getErrors()) {
    std::cerr << "Ligne " << err.line_number
              << ": " << err.message << "\n";
  }
    ```
  
### Order
- Structure data pour un ordre :  
  `timestamp, order_id, instrument, side, type, quantity, price, action`
- Usines : `Order::makeLimit(...)`, `Order::makeMarket(...)`
- Validation interne (`validate()`) pour garantir `qty > 0` et `price > 0` pour les LIMIT
- `toString()` & `operator<<` pour le debugging

### MatchResult
- Contient :  
  `timestamp, order_id, instrument, side, type, quantity restante, price, action, status, executed_quantity, execution_price, counterparty_id`
- Enum `Status` : `PENDING`, `EXECUTED`, `PARTIALLY_EXECUTED`, `CANCELED`, `REJECTED`
- `toString(Status)` pour CSV et logs

### OrderBook
- Carnet FIFO par prix, deux `std::map<double, std::deque<Order>>` (sell asc, buy desc)
- Méthodes :
    - `process(const Order&)` → route vers `addLimitOrder` / `cancelOrder` / `matchLimit` / `matchMarket`
    - `addLimitOrder()`, `cancelOrder()`, `matchLimit()`, `matchMarket()`

### MatchingEngine
- Orchestrateur principal :
    1. Bookkeeping (`originalQty_`, `remainingQty_`)
    2. Délégation à `OrderBook` par instrument
    3. Conversion de chaque `Execution` en `MatchResult` (avec `status`)
    4. Ajout d’un `MatchResult` PENDING/CANCELED s’il n’y a pas de fill

### Logger
- Logging métier : `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`
- Horodatage millisecondes + niveau + message
- Flag runtime `me::setLoggingEnabled(bool)` pour désactiver en bench/tests perf

---

## IV - Exécution des tests unitaires

1. **Configurer, compiler et tester**
   ```bash
   mkdir -p build
   cd build
   cmake ..
   cmake --build .
   ctest -N
   ctest --output-on-failure
    ```
### Préparation automatique des données

- Les fichiers CSV situés dans `tests/data` sont copiés dans `build/tests/data` avant l’exécution de chaque test.

### Couverture testée

- **CsvParser** : parsing, gestion des erreurs, saut d’en-tête
- **CsvWriter** : écriture du header et des `MatchResult`
- **OrderBook** : insertions, annulations, matching `limit` & `market`
- **MatchingEngine** : orchestration `NEW`/`MODIFY`/`CANCEL`, conversion en `MatchResult`
- **Test de throughput unitaire** (`test_Performance.cpp`) : insertion de N ordres et mesure du temps CPU

## V - Bench de performance

Pour mesurer le débit pur du moteur (insertion + matching):

1. **Désactiver les logs**
   ```cpp
   me::setLoggingEnabled(false);
   ```
   
2. **Exécuter le bench à l'aide simulation d'ordres**
   ```bash
     cd build
     cmake --build .
     ./Performance
     ```
3. **Exemple de sortie**
    ```
    Processed 500000 orders in 1.50 s → 333 k ops/s
    ```
- Affiche le temps pour traiter 500 000 ordres et le débit en opérations par seconde.
- Seule la méthode MatchingEngine::process() est chronométrée.
