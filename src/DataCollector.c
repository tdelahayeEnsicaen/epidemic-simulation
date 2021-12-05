#include "DataCollector.h"
#include "Map.h"

#include <stdlib.h>
#include <stdio.h>

#include <ncurses.h>

#include <unistd.h>

FILE* pFile;
WINDOW* pLegendWindow;
WINDOW* pTilesWindow;
WINDOW* pCitizensWindow;
WINDOW* pProgressionWindow;

#define DEFAULT_COLOR 1
#define HOUSE_COLOR 2
#define HOSPITAL_COLOR 3
#define WASTELAND_COLOR 4
#define FIRE_STATION_COLOR 5

int turnCounter = 0;

void initDataCollector()
{
    pFile = fopen("evolution.txt", "w");

    if (!pFile)
    {
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    fprintf(pFile, "Turn Healthy Sick Dead Burned\n");

    int aliveCount = 0;
    int sickCount = 0;
    int deadCount = 0;
    int burnCount = 0;

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        const Citizen* pCitizen = getCitizen(i);

        aliveCount += pCitizen->status == HEALTHY;
        sickCount += pCitizen->status == SICK;
        deadCount += pCitizen->status >= DEAD;
        burnCount += pCitizen->status == BURNED;
    }

    fprintf(pFile, "%d %d %d %d %d\n", turnCounter, aliveCount, sickCount, deadCount, burnCount);

    initscr();

    start_color();

    init_pair(DEFAULT_COLOR, COLOR_BLUE, COLOR_WHITE);
    init_pair(HOUSE_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(HOSPITAL_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(WASTELAND_COLOR, COLOR_WHITE, COLOR_YELLOW);
    init_pair(FIRE_STATION_COLOR, COLOR_WHITE, COLOR_RED);

    curs_set(0);

    noecho();

    pLegendWindow = subwin(stdscr, 9, 23, 1, 2);
    pTilesWindow = subwin(stdscr, 9, 16, 1, 27);
    pCitizensWindow = subwin(stdscr, 9, 18, 1, 45);
    pProgressionWindow = subwin(stdscr, 10, 61, 10, 2);
}

int getColor(char type)
{
    switch (type)
    {
    case HOSPITAL:
        return HOSPITAL_COLOR;
    
    case FIRE_STATION:
        return FIRE_STATION_COLOR;

    case HOUSE:
        return HOUSE_COLOR;

    case WASTELAND:
        return WASTELAND_COLOR;
    
    default:
        return 0;
    }
}

void writeWithAttribute(WINDOW* pWindow, int x, int y, int attribute, const char* text)
{
    wattron(pWindow, attribute);
    mvwprintw(pWindow, x, y, text);
    wattroff(pWindow, attribute);
}

void colorTile(WINDOW* pWindow, int x, int y, int color)
{
    writeWithAttribute(pWindow, x, y, COLOR_PAIR(color), "  ");
}

void updateDataCollector()
{
    clear();
    bkgd(COLOR_PAIR(DEFAULT_COLOR));
    refresh();

    drawLegendWindow();
    drawTilesWindow();
    drawCitizensWindow();
    drawProgressionWindow();
}

void destroyDataCollector()
{
    fclose(pFile);

    createPlot();

    getch();
    endwin();

    curs_set(1);
    echo();

    /*free(pLegendWindow);
    free(pTilesWindow);
    free(pCitizensWindow);
    free(pProgressionWindow);*/
}

void drawLegendWindow()
{
    box(pLegendWindow, ACS_VLINE, ACS_HLINE);

    wbkgd(pLegendWindow, COLOR_PAIR(DEFAULT_COLOR));

    writeWithAttribute(pLegendWindow, 0, 0, A_UNDERLINE, "Legende");

    colorTile(pLegendWindow, 1, 0, HOUSE_COLOR);
    colorTile(pLegendWindow, 3, 0, HOSPITAL_COLOR);
    colorTile(pLegendWindow, 5, 0, WASTELAND_COLOR);
    colorTile(pLegendWindow, 7, 0, FIRE_STATION_COLOR);

    mvwprintw(pLegendWindow, 1, 3, "Maison");
    mvwprintw(pLegendWindow, 3, 3, "Hopital");
    mvwprintw(pLegendWindow, 5, 3, "Terrain vague");
    mvwprintw(pLegendWindow, 7, 3, "Caserne de pompiers");

    wrefresh(pLegendWindow);
}

void drawTilesWindow()
{
    box(pTilesWindow, ACS_VLINE, ACS_HLINE);

    wbkgd(pTilesWindow, COLOR_PAIR(DEFAULT_COLOR));

    writeWithAttribute(pTilesWindow, 0, 0, A_UNDERLINE, "Carte des lieux");

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            const Tile tile = getTile(x, y);

            colorTile(pTilesWindow, 1 + y, 2 * x + 1, getColor(tile.type));
        }
    }

    wrefresh(pTilesWindow);
}

void drawCitizensWindow()
{
    box(pCitizensWindow, ACS_VLINE, ACS_HLINE);

    wbkgd(pCitizensWindow, COLOR_PAIR(DEFAULT_COLOR));

    writeWithAttribute(pCitizensWindow, 0, 0, A_UNDERLINE, "Carte des citoyens");

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            int count = getCitizenCount(x, y);

            mvwprintw(pCitizensWindow, 1 + y, 2 * x + 2, "%d ", count);
        }
    }
    
    wrefresh(pCitizensWindow);
}

void drawProgressionWindow()
{
    turnCounter++;
    box(pProgressionWindow, ACS_VLINE, ACS_HLINE);

    wbkgd(pProgressionWindow, COLOR_PAIR(DEFAULT_COLOR));

    writeWithAttribute(pProgressionWindow, 0, 0, A_UNDERLINE, "Evolution de l'epidemie");

    int aliveCount = 0;
    int sickCount = 0;
    int deadCount = 0;
    int burnCount = 0;

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        const Citizen* pCitizen = getCitizen(i);

        aliveCount += pCitizen->status == HEALTHY;
        sickCount += pCitizen->status == SICK;
        deadCount += pCitizen->status >= DEAD;
        burnCount += pCitizen->status == BURNED;
    }

    mvwprintw(pProgressionWindow, 2, 0, "Personnes en bonne sante  %d", aliveCount);
    mvwprintw(pProgressionWindow, 4, 0, "Personnes malades         %d", sickCount);
    mvwprintw(pProgressionWindow, 6, 0, "Personnes decedees        %d", deadCount);
    mvwprintw(pProgressionWindow, 8, 0, "Personnes brules          %d", burnCount);
    mvwprintw(pProgressionWindow, 5, 35, "TOUR %d/100", turnCounter);

    wrefresh(pProgressionWindow);

    fprintf(pFile, "%d %d %d %d %d\n", turnCounter, aliveCount, sickCount, deadCount, burnCount);
}

void createPlot()
{
    int pid = fork();

    if (pid == -1)
    {
        printf("Erreur: échec du fork()\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        execlp("gnuplot", "gnuplot","-persist", "plot.gp", NULL);
        exit(EXIT_FAILURE);
    }
}