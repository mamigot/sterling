#ifndef REPLICATION_H_
#define REPLICATION_H_

void requestHandler(const unsigned int connfd, const unsigned int recvPort);

void configReplication();

void launchElections();

void launch();

#endif
