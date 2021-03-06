#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <map>
#include <list>
#include <queue>
#include <ctime>
#include <random>

using namespace std;

std::mt19937 rng(177013);

int chessBoardSize;
int fieldSize;
int initialPopulationCount = 10;

enum Mode 
{
    ASSOCIATIVE,
    COMPOSITE,
    MULTIMAGIC
};

struct individual
{
    int* arrangement;
    int cost;

    individual(){}

    individual(individual& copy)
    {
        arrangement = new int[fieldSize];
        for (int i = 0; i < fieldSize; i++)
        {
            arrangement[i] = copy.arrangement[i];
        }
        cost = copy.cost;
    }

    ~individual()
    {
        delete[] arrangement;
    }

};

typedef vector<individual*> population_type;

population_type population;

//simulation mode
Mode simMode=Mode::ASSOCIATIVE;

//maximum power for multimagic square
int P;
//dimension within subsquare in composite square
int n;
//dimension of subsquares in composite square, m*m number of subsquares
int m;

//calculates fitness, low value is good
unsigned int fitnessValue(int arrangement[])
{
    unsigned int fitness = 0;

    int magicValue = 0;
    int sum = 0;

    switch (simMode)
    {
    // Associative magic square
    case ASSOCIATIVE:
    {
        // Check first for classic magic square
        // Calculate magic number 
        magicValue = chessBoardSize * (fieldSize + 1) / 2;

        // Compute difference to magic number for each row
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum = 0;
            for (int j = 0; j < chessBoardSize; j++)
            {
                sum += arrangement[i * chessBoardSize + j];
            }
            fitness += std::abs(sum - magicValue);
        }

        // Compute difference to magic number for each column
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum = 0;
            for (int j = 0; j < chessBoardSize; j++)
            {
                sum += arrangement[j * chessBoardSize + i];
            }
            fitness += std::abs(sum - magicValue);
        }

        // Compute difference to magic number for both diagonals
        sum = 0;
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum += arrangement[i * chessBoardSize + i];
        }
        fitness += std::abs(sum - magicValue);

        sum = 0;
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum += arrangement[i * chessBoardSize + (chessBoardSize - i - 1)];
        }
        fitness += std::abs(sum - magicValue);

        // Check if square is associative
        magicValue = fieldSize + 1;

        for (int i = 0; i < std::ceil(fieldSize/2); i++)
        {
            fitness += std::abs(arrangement[i] + arrangement[fieldSize - i - 1] - magicValue);
        }
    }
        break;
    // Composite magic square
    case COMPOSITE:
    {
        int* tempSub = new int[n * n];
        int subMin;

        // Calculate magic number for subsquares
        magicValue = n * (n * n + 1) / 2;

        // Calculate fitness of subsquares
        // .--->x
        // |012
        // |345
        // |678
        // V y
        for (int my = 0; my < m; my++)
        {
            for (int mx = 0; mx < m; mx++)
            {
                // Write into temp array and find minimum value
                subMin = INT32_MAX;

                for (int ny = 0; ny < n; ny++)
                {
                    for (int nx = 0; nx < n; nx++)
                    {
                        tempSub[ny * n + nx] = arrangement[(my * n + ny) * (n * m) + (mx * n + nx)];
                        subMin = std::min(subMin, tempSub[ny * n + nx]);
                    }
                }

                // Transform temp array to have lowest value = 1
                for (int i = 0; i < n * n; i++)
                {
                    tempSub[n] -= subMin - 1;
                }

                // Check subsquare for classic magic square
                // Compute difference to magic number for each row
                for (int i = 0; i < n; i++)
                {
                    sum = 0;
                    for (int j = 0; j < n; j++)
                    {
                        sum += tempSub[i * n + j];
                    }
                    fitness += std::abs(sum - magicValue);
                }

                // Compute difference to magic number for each column
                for (int i = 0; i < n; i++)
                {
                    sum = 0;
                    for (int j = 0; j < n; j++)
                    {
                        sum += tempSub[j * n + i];
                    }
                    fitness += std::abs(sum - magicValue);
                }

                // Compute difference to magic number for both diagonals
                sum = 0;
                for (int i = 0; i < n; i++)
                {
                    sum += tempSub[i * n + i];
                }
                fitness += std::abs(sum - magicValue);

                sum = 0;
                for (int i = 0; i < n; i++)
                {
                    sum += tempSub[i * n + (n - i - 1)];
                }
                fitness += std::abs(sum - magicValue);
            }
        }

        // Check full square for classic magic square
        // Calculate magic number 
        magicValue = chessBoardSize * (fieldSize + 1) / 2;

        // Compute difference to magic number for each row
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum = 0;
            for (int j = 0; j < chessBoardSize; j++)
            {
                sum += arrangement[i * chessBoardSize + j];
            }
            fitness += std::abs(sum - magicValue);
        }

        // Compute difference to magic number for each column
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum = 0;
            for (int j = 0; j < chessBoardSize; j++)
            {
                sum += arrangement[j * chessBoardSize + i];
            }
            fitness += std::abs(sum - magicValue);
        }

        // Compute difference to magic number for both diagonals
        sum = 0;
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum += arrangement[i * chessBoardSize + i];
        }
        fitness += std::abs(sum - magicValue);

        sum = 0;
        for (int i = 0; i < chessBoardSize; i++)
        {
            sum += arrangement[i * chessBoardSize + (chessBoardSize - i - 1)];
        }
        fitness += std::abs(sum - magicValue);

        delete[] tempSub;
        }
        break;
    // Multimagic square
    case MULTIMAGIC: // CAUTION! will certainly cause overflow errors when testing with high orders and/or powers. (n*n)^P < int_max as a first estimate
    {
        int* temp = new int[fieldSize];
        std::fill_n(temp, fieldSize, 1);

        
        for (int k = 1; k <= P; k++)
        {
            // Calculate k-th power of array element, save in temp array
            for (int i = 0; i < fieldSize; i++)
            {
                temp[i] *= arrangement[i];
            }

            // Calculate magic number for current power, formula source: http://www.multimagie.com/English/Formula.htm
            switch (k)
            {
            case 1:
                magicValue = chessBoardSize * (fieldSize + 1) / 2;
                break;
            case 2:
                magicValue = (chessBoardSize * (fieldSize + 1) / 2) * (2*fieldSize + 1) / 3;
                break;
            case 3:
                magicValue = chessBoardSize * (chessBoardSize * (fieldSize + 1) / 2) * (chessBoardSize * (fieldSize + 1) / 2);
                break;
            case 4:
                magicValue = ((chessBoardSize * (fieldSize + 1) / 2) * (2 * fieldSize + 1) / 3) * (6 * chessBoardSize * (chessBoardSize * (fieldSize + 1) / 2) - 1) / 5;
                break;
            case 5:
                magicValue = (3 * chessBoardSize * (((chessBoardSize * (fieldSize + 1) / 2) * (2 * fieldSize + 1) / 3) * ((chessBoardSize * (fieldSize + 1) / 2) * (2 * fieldSize + 1) / 3)) - (chessBoardSize * (chessBoardSize * (fieldSize + 1) / 2) * (chessBoardSize * (fieldSize + 1) / 2))) / 2;
                break;
            default:
                std::cout << "Unsupported power of " << k << " for multimagic square!" << std::endl;
                break;
            }

            // Compute difference to magic number for each row
            for (int i = 0; i < chessBoardSize; i++)
            {
                sum = 0;
                for (int j = 0; j < chessBoardSize; j++)
                {
                    sum += temp[i * chessBoardSize + j];
                }
                fitness += std::abs(sum - magicValue);
            }

            // Compute difference to magic number for each column
            for (int i = 0; i < chessBoardSize; i++)
            {
                sum = 0;
                for (int j = 0; j < chessBoardSize; j++)
                {
                    sum += temp[j * chessBoardSize + i];
                }
                fitness += std::abs(sum - magicValue);
            }

            // Compute difference to magic number for both diagonals
            sum = 0;
            for (int i = 0; i < chessBoardSize; i++)
            {
                sum += temp[i * chessBoardSize + i];
            }
            fitness += std::abs(sum - magicValue);

            sum = 0;
            for (int i = 0; i < chessBoardSize; i++)
            {
                sum += temp[i * chessBoardSize + (chessBoardSize - i - 1)];
            }
            fitness += std::abs(sum - magicValue);
        }

        delete[] temp;
    }
        break;
    default:
    {
        std::cout << "Unsupported mode!" << std::endl;
        fitness = INT32_MAX;
    }
        break;
    }

    return fitness;
}

individual* createNode()
{
    individual* newNode = new individual;
    newNode->arrangement = new int[fieldSize];
    return newNode;
}

// Returns a random value between Min and Max (both inclusive)
int random(int min, int max)
{
    // Mersenne-Twister
    double r = (double)max - (double)min + 1;
    return min + (int)(r * rng() / (std::mt19937::max() + 1.0));
}

void generatePopulation()
{
    const int arraySize = chessBoardSize * chessBoardSize;
    int* sampleArrangement = new int[arraySize];
    individual* temp;
    for (int i = 0; i < arraySize; i++)
    {
        sampleArrangement[i] = i + 1;
    }

    // Adds entries to population list
    for (int i = 0; i < initialPopulationCount; i++)
    {
        // Permute In Place (Random Shuffle)
        for (int j = 0; j < arraySize; j++)
        {
            std::swap(sampleArrangement[j], sampleArrangement[random(j, arraySize-1)]);
        }
        temp = createNode();
        for (int j = 0; j < arraySize; j++)
        {
            temp->arrangement[j] = sampleArrangement[j];
        }
        temp->cost = fitnessValue(sampleArrangement);
        population.push_back(temp);
    }

    delete[] sampleArrangement;
}

// Creates an array in which the number at a certain index indicates 
// how many numbers in the converted array (which were left of the number 
// of the index position) were greater than the number itself, formula source: https://user.ceng.metu.edu.tr/~ucoluk/research/publications/tspnew.pdf  
int* createInversionSequence(individual* individual)
{
    int* inversionSequence = new int[fieldSize];

    for (int i = 0; i < fieldSize; i++)
    {
        int counter = 0;

        for(int j = 0; j < fieldSize; j++)
        {
            if (individual->arrangement[j] == i + 1)
                break;

            if (individual->arrangement[j] > i + 1)
                counter++;
        }
        inversionSequence[i] = counter;
    }

    return inversionSequence;
}

// Inverse function of "createInversionSequence()"
int* recreateNumbers(int* inversionSequence)
{
    int* positions = new int[fieldSize];
    int* square = new int[fieldSize];

    for (int i = (fieldSize - 1); i >= 0; i--)
    {
        int additionValue = 1;
        int currentValue = inversionSequence[i];
        positions[i] = currentValue;

        while (i + additionValue <= (fieldSize - 1))
        {
            if (positions[i + additionValue] >= positions[i])
            {
                positions[i + additionValue]++;
            }

            additionValue++;
        }
    }

    for (int i = 0; i < fieldSize; i++)
    {
        int insertPosition = positions[i];
        square[insertPosition] = (i + 1);
    }

    delete[] positions;

    return square;
}

individual* mutate(individual* child)
{
    // Chance to mutate more cells than just 2
    for (int i = 0; random(0, 100) < 50; i++)
    {
        // Swap to random cells
        int pos1 = random(0, fieldSize - 1);
        int pos2 = random(0, fieldSize - 1);

        while (pos1 == pos2)
        {
            pos2 = random(0, fieldSize - 1);
        }

        int buffer = child->arrangement[pos2];
        child->arrangement[pos2] = child->arrangement[pos1];
        child->arrangement[pos1] = buffer;
    }

    return child;
}

individual* reproduce(individual* parent1, individual* parent2)
{
    individual* child = createNode();

    // Create a sequence for each parent
    int* inversionSequenceP1;
    inversionSequenceP1 = createInversionSequence(parent1);     // "Encode the numbers"

    int* inversionSequenceP2;
    inversionSequenceP2 = createInversionSequence(parent2);     // "Encode the numbers"

    int min = fieldSize * 0.2;
    int max = fieldSize * 0.8;
    int crossoverPoint = random(min, max) + 1;  // Randomly select crossover point

    int* inversionSequenceChild = new int[fieldSize];
    std::copy(inversionSequenceP1, inversionSequenceP1 + crossoverPoint, inversionSequenceChild);
    std::copy(inversionSequenceP2 + crossoverPoint, inversionSequenceP2 + fieldSize, inversionSequenceChild + crossoverPoint );

    delete[] child->arrangement;
    child->arrangement = recreateNumbers(inversionSequenceChild);   // "Decode the sequence to get the numbers"

    delete[] inversionSequenceP1;
    delete[] inversionSequenceP2;
    delete[] inversionSequenceChild;

    return child;
}

int randomSelection()
{
    // Randomly select one from the best half
    int randomPos = random(0, population.size()) / 2;
    return randomPos;
}

bool isFit(individual* test)
{
    if (test->cost == 0)
        return true;
    return false;
}

bool comp(individual* a, individual* b)
{
    return(a->cost < b->cost);
}

individual* GA()
{
    int randomNum1 = 0;
    int randomNum2 = 0;
    individual* individualX = nullptr;
    individual* individualY = nullptr;
    individual* child = nullptr;

    bool found = false;
    int bestCost = INT32_MAX;
    int noImprovmentSince = 0;
    while (!found)
    {
        // Sort Population (best are at begin)
        sort(population.begin(), population.end(), comp);

        // Check if generation is better or not
        if (bestCost > population[0]->cost)
        {
            bestCost = population[0]->cost;
            noImprovmentSince = 0;
        }
        else
        {
            noImprovmentSince++;
        }

        // If there were no improvemnts since x generations, this attempt dies
        if (noImprovmentSince > 10000)
        {
            //std::cout << "Go back to monkee!" << std::endl;
            return nullptr;
        }

        population_type new_population;
        for (unsigned int i = 0; i < population.size()/2; i++)
        {
            new_population.push_back(new individual(*population[i]));
            // Select Parents
            randomNum1 = randomSelection();
            individualX = population[randomNum1];

            randomNum2 = randomSelection();
            individualY = population[randomNum2];

            // Produce Child
            child = reproduce(individualX, individualY);

            // Mutations chance is handled in mutate
            child = mutate(child);

            // Calculate FitnessValue
            child->cost = fitnessValue(child->arrangement);

            if (child->cost==0)     // found correct solution
            {
                found = 1;
                while (population.size() > 0)
                {
                    delete population.back();
                    population.pop_back();
                }
                population = new_population;
                new_population.clear();
                return child;
            }
            new_population.push_back(child);
        }
        // Population is set to new generation
        while (population.size() > 0)
        {
            delete population.back();
            population.pop_back();
        }
        population = new_population;
        new_population.clear();
    }
    return child;
}

char* getOption(char** start, char** end, const std::string& option)
{
    char** iterator = std::find(start, end, option);

    if (iterator != end && ++iterator != end)
    {
        return *iterator;
    }
    return 0;

}

bool OptionExists(char** start, char** end, const std::string& option)
{
    return std::find(start, end, option) != end;
}

int main(int argc, char** argv)
{
    int maxSolutions = 5, numFound = 0;

    rng.seed(clock());  // Random Seed

    //input
    if (OptionExists(argv, argv + argc, "--size")) //size in 1 dimension
    {
        chessBoardSize = std::atoi(getOption(argv, argv + argc, "--size"));
        fieldSize = chessBoardSize * chessBoardSize;
    }
    if (OptionExists(argv, argv + argc, "--power")) //power for the multimagic square
    {
        P = std::atoi(getOption(argv, argv + argc, "--power"));
    }
    if (OptionExists(argv, argv + argc, "--n")) //n of composite magic square
    {
        n = std::atoi(getOption(argv, argv + argc, "--n"));
    }
    if (OptionExists(argv, argv + argc, "--m")) //m of composite magic square
    {
        m = std::atoi(getOption(argv, argv + argc, "--m"));
    }
    if (OptionExists(argv, argv + argc, "--mode")) //mode of square (for more info see enum Mode)
    {
        simMode = static_cast<Mode>(std::atoi(getOption(argv, argv + argc, "--mode")));
    }
    if (OptionExists(argv, argv + argc, "--num")) //maximum number of solutions
    {
        maxSolutions = std::atoi(getOption(argv, argv + argc, "--num"));
    }

    //output
    clock_t start_time, end_time;           //to keep a track of the time spent in computing
    map<string, bool> solutionsFound;
    start_time = clock();
    std::cout << "Magic Square Solver" << std::endl << std::endl;
    while (numFound != maxSolutions)
    {
        generatePopulation();
        individual* solution = GA();
        if (solution != nullptr)
        {
            string hash = ""; //generate string from solution, to save it in a map
            for (int i = 0; i < chessBoardSize * chessBoardSize; i++)
            {
                hash = hash + std::to_string(solution->arrangement[i]);
            }

            // Print solutions
            if (!solutionsFound[hash])
            {
                solutionsFound[hash] = true;
                std::cout << "Possible Solution #" << (++numFound) << ":\t" << std::endl;
                for (int x = 0; x < chessBoardSize; x++)
                {
                    for (int y = 0; y < chessBoardSize; y++)
                    {
                        std::cout << solution->arrangement[x * chessBoardSize + y] << "\t";
                    }
                    std::cout << std::endl;
                }
            }
            delete solution;
        }
        while (population.size() > 0)
        {
            delete population.back();
            population.pop_back();
        }
    }
    end_time = clock();

    cout << "Time required for execution: \t" << 1000 * ((double)(end_time - start_time) / CLOCKS_PER_SEC) << " milliseconds." << "\n\n";
    return 0;
}
