#ifndef ERREURS_H
#define ERREURS_H


#include "modele.hpp"


namespace utils {


struct Erreur
{
  unsigned int id;
  utils::model::Localized locale;
};

extern int erreurs_charge();
extern int erreur_affiche(unsigned int id);
extern Erreur &erreur_get(unsigned int);


extern void signale_erreur(unsigned int id, ...);

extern void affiche_pile_erreurs();

}





#endif

