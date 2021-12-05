#include "DataCollector.h"
#include "Map.h"

#include <stdlib.h>
#include <stdio.h>

#include <ncurses.h>

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

void initDataCollector()
{
    pFile = fopen("evolution.txt", "w");

    if (!pFile)
    {
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    saveMap(pFile);

    initscr();

    start_color();

    init_pair(DEFAULT_COLOR, COLOR_BLUE, COLOR_WHITE);
    init_pair(HOUSE_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(HOSPITAL_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(WASTELAND_COLOR, COLOR_WHITE, COLOR_YELLOW);
    init_pair(FIRE_STATION_COLOR, COLOR_WHITE, COLOR_RED);

    curs_set(0);

    noecho();

    pLegendWindow = subwin(stdscr, 9, 22, 2, 2);
    pTilesWindow = subwin(stdscr, 9, 15, 2, 27);
    pCitizensWindow = subwin(stdscr, 9, 18, 2, 45);
    pProgressionWindow = subwin(stdscr, 9, 28, 12, 2);
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

    drawLegendWindow();
    drawTilesWindow();
    drawCitizensWindow();
    drawProgressionWindow();

    refresh();

    saveMap(pFile);
}

void destroyDataCollector()
{
    fclose(pFile);

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

    colorTile(pLegendWindow, 2, 0, HOUSE_COLOR);
    colorTile(pLegendWindow, 4, 0, HOSPITAL_COLOR);
    colorTile(pLegendWindow, 6, 0, WASTELAND_COLOR);
    colorTile(pLegendWindow, 8, 0, FIRE_STATION_COLOR);

    mvwprintw(pLegendWindow, 2, 3, "Maison");
    mvwprintw(pLegendWindow, 4, 3, "Hopital");
    mvwprintw(pLegendWindow, 6, 3, "Terrain vague");
    mvwprintw(pLegendWindow, 8, 3, "Caserne de pompiers");

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

            colorTile(pTilesWindow, 2 + y, 2 * x, getColor(tile.type));
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

    wrefresh(pProgressionWindow);
}