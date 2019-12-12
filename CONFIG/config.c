/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES
/*
 * Indications (à respecter) :
 * - Les erreurs sont gérées avec des assert ; les erreurs traitées sont :
 *    . appel trop tôt ou trop tard d'une méthode (cf. config.h)
 *    . fichier de configuration inaccessible
 *    . une position erronnée
 * - Le fichier (si on arrive à l'ouvrir) est considéré comme bien
 *   formé sans qu'il soit nécessaire de le vérifier
 *
 * Un code minimal est fourni et permet d'utiliser le module "config" dès
 * le début du projet ; il faudra le remplacer par l'utilisation du fichier
 * de configuration.
 * Il est inutile de faire plus que ce qui est demandé
 *
 * Dans cette partie vous avez le droit d'utiliser les entrées-sorties
 * de haut niveau (fopen, fgets, ...)
 */


// TODO include des .h système
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "config.h"

#define  EXIT_INIT -2
#define  EXIT_EXIT -3
#define  EXIT_POS -4
#define  MAX_LENGTH_OPEN strlen("ouvert") + 1
#define  MAX_LENGTH_NAME strlen("../SERVICES/service_compression") + 1

// TODO Structure de données ici
typedef struct{
  FILE *file;
  int nbServices;
  char **services;
  bool *isOpen;
} Fichier;

static bool init = false;
static bool configExit = false;
Fichier fichier;

//---------------------------------------------------------------------------------
static void readFile(){

  if(fscanf(fichier.file, "%d", &fichier.nbServices) != 1)
  {
    perror("");
    exit(EXIT_FAILURE);
  }

  //allouer 2 tableau avec le nbServices comme taille
  // le premier qui determine si chaque service est ouvert
  // le deuxieme avec le nom de chaque service
  fichier.isOpen = malloc(sizeof(bool) * fichier.nbServices);
  fichier.services = malloc(sizeof(char *) * fichier.nbServices);


  char * isOpen = malloc( MAX_LENGTH_OPEN );
  char * name = malloc( MAX_LENGTH_NAME );
  for (int i = 0; i < fichier.nbServices; i++)
  {
    //lire d'abord si le fichier est ouvert
    if(fscanf(fichier.file, "%s", isOpen) != 1)
    {
      perror("");
      exit(EXIT_FAILURE);
    }

    if (strcmp(isOpen, "ouvert") == 0)
    {
      fichier.isOpen[i] = true;
    }else{
      fichier.isOpen[i] = false;
    }


    //lire le nom du fichier
    if(fscanf(fichier.file, "%s", name) != 1)
    {
      perror("");
      exit(EXIT_FAILURE);
    }
    fichier.services[i] = malloc(strlen(name) + 1);
    strcpy(fichier.services[i], name);
  }

  free(isOpen);
  free(name);
}

//---------------------------------------------------------------------------------
static void closeFile()
{
  for (int i = 0; i < fichier.nbServices; i++)
  {
    free(fichier.services[i]);
  }

  fichier.nbServices = 0;
  free(fichier.services);
  free(fichier.isOpen);
  fclose(fichier.file);
}


//---------------------------------------------------------------------------------
void config_init(const char *filename)
{
    // TODO erreur si la fonction est appelée deux fois
    if (init)
    {
      exit(EXIT_INIT);
    }
    else
    {
      fichier.file = fopen(filename, "r");
      assert(fichier.file != NULL);
      readFile();
      //eviter d'appeler cette fonction plus d'une fois
      init = true;
      //on peut appeler la fonction config_exit
      configExit = false;
    }
}

//---------------------------------------------------------------------------------
void config_exit()
{
    // TODO erreur si la fonction est appelée avant config_init
    if (init && !configExit)
    {
        // on a reussi à appeler cette fonction une seule fois
        closeFile();

        //permettre appeler config_init une autre fois
        init = false;
        configExit = true;
    }
    else
    {
      exit(EXIT_EXIT);
    }
}

//---------------------------------------------------------------------------------
int config_getNbServices()
{
    // erreur si la fonction est appelée avant config_init
    if (!init) {
      exit(EXIT_INIT);
    }
    // erreur si la fonction est appelée après config_exit
    if (configExit) {
      exit(EXIT_EXIT);
    }

    // code par défaut, à remplacer
    return fichier.nbServices;
}

//---------------------------------------------------------------------------------
bool config_isServiceOpen(int pos)
{
    // erreur si la fonction est appelée avant config_init
    if (!init)
    {
      exit(EXIT_INIT);
    }
    // erreur si la fonction est appelée après config_exit
    if (configExit)
    {
      exit(EXIT_EXIT);
    }
    // TODO erreur si "pos" est incorrect
    if (pos < 0 || pos > fichier.nbServices)
    {
      exit(EXIT_POS);
    }

    return fichier.isOpen[pos];
}

//---------------------------------------------------------------------------------
const char * config_getExeName(int pos)
{
  // erreur si la fonction est appelée avant config_init
  if (!init)
  {
    exit(EXIT_INIT);
  }
  // erreur si la fonction est appelée après config_exit
  if (configExit)
  {
    exit(EXIT_EXIT);
  }
  // TODO erreur si "pos" est incorrect
  if (pos < 0 || pos > fichier.nbServices)
  {
    exit(EXIT_POS);
  }

    return fichier.services[pos];
}
