#include "storage/config.h"
#include "replication.h"


int main(int argc, char **argv) {
  configStorage();
  configReplication();

  launch();
}
