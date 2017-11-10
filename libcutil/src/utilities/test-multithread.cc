

class Essai
{
public:

  utils::model::Node noeud;

  unsigned int cnt[2];

  utils::hal::Signal sig[2];

  Essai()
  {
    cnt[0] = cnt[1] = 0;
  }

  void essai1()
  {
    for(;;)
    {
      noeud.get_attribute_as_int("config/general/langue");
      cnt[0]++;
      if(cnt[0] > 500000)
      {
        utils::infos("Fin thread 1");
        sig[0].raise();
        return;
      }
    }
  }

  void essai2()
  {
    for(;;)
    {
      noeud.set_attribute("config/general/langue", (int) cnt[1]);
      cnt[1]++;
      if(cnt[1] > 500000)
      {
        utils::infos("Fin thread 2");
        sig[1].raise();
        return;
      }
    }
  }
};


int main()
{
  Essai essai;
  essai.noeud = utils::model::Node::create_ram_node(mgc_schema);
  utils::infos("Début test concurrence et modèle...");
  utils::hal::thread_start(&essai, &Essai::essai1);
  utils::hal::thread_start(&essai, &Essai::essai2);

  essai.sig[0].wait();
  essai.sig[1].wait();
}



