#include "Reactor.h"

void *createReactor()
{
	preactor react = NULL;

	if ((react = (preactor)malloc(sizeof(reactor))) == NULL)
	{
		perror("malloc() failed");
		exit(-1);
	}

	react->hot = true;
	react->clients_counter = 0;
	react->fd_size = 4;
	react->thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	react->pfds = (struct pollfd *)malloc(sizeof(struct pollfd) * 4);
	if (!react->pfds)
	{
		perror("malloc pfds() failed");
		free(react);
		exit(-1);
	}
	// hashmap
	react->FDtoFunction = hashmap_create();
	if (!react->FDtoFunction)
	{
		free(react->pfds);
		free(react);
		printf("hashmap_create() failed\n");
	}

	fprintf(stdout, "Reactor created.\n");
	return react;
}

void *reactorRun(void *this)
{
	if (this == NULL)
	{
		perror("reactorRun() failed");
		exit(-1);
	}

	preactor reactor = (preactor)this;

	while (reactor->hot)
	{
		if (poll(reactor->pfds, reactor->clients_counter, 1000) == -1)
		{
			perror("poll() failed");
			exit(-1);
		}

		for (size_t i = 0; i < reactor->clients_counter; i++)
		{
			uintptr_t function;
			if (reactor->pfds[i].revents & POLLIN)
			{
				// calling hashmap function
				if (!hashmap_get(reactor->FDtoFunction, &reactor->pfds[i].fd, sizeof(int), &function))
				{
					printf("hashmap_get() failed\n");
					continue;
				}

				handler_t a = (handler_t)function;
				if (a(reactor->pfds[i].fd, reactor) == -1)
				{
					printf("client's function failed\n");
				}
			}
		
		}
	}

	fprintf(stdout, "Reactor thread finished.\n");

	return reactor;
}

void startReactor(void *this)
{
	if (this == NULL)
	{
		perror("startReactor() failed");
		exit(-1);
	}

	preactor react = (preactor)this;

	if (react->hot)
		return;
	else
	{
		react->hot = true;
		pthread_create(&react->thread, NULL, reactorRun, this);
		fprintf(stdout, "Reactor thread started.\n");
	}
}

void stopReactor(void *this)
{
	if (this == NULL)
	{
		perror("stopReactor() failed");
		exit(-1);
	}

	preactor reactor = (preactor)this;

	if (!reactor->hot)
		return;

	reactor->hot = false;

	pthread_cancel(reactor->thread);
	pthread_join(reactor->thread, NULL);
	pthread_detach(reactor->thread);

	fprintf(stdout, "Reactor thread stopped and detached.\n");
}

void addFd(void *this, int fd, handler_t handler)
{
	if (this == NULL)
	{
		errno = EINVAL;
		perror("addFd() failed");
		return;
	}

	preactor reactor = (preactor)this;

	if (reactor->clients_counter == reactor->fd_size)
	{
		reactor->fd_size *= 2; // Double it

		reactor->pfds = realloc(reactor->pfds, sizeof(struct pollfd) * (reactor->fd_size));

		if (!reactor->pfds)
		{
			perror("realloc() failed");
			free(reactor);
			return;
		}
	}

	reactor->pfds[reactor->clients_counter].fd = fd;
	reactor->pfds[reactor->clients_counter].events = POLLIN;
	reactor->clients_counter++;
	int *fdcpy = (int *)malloc(sizeof(int));
	*fdcpy = fd;
	// add fd to hashmap
	hashmap_set(reactor->FDtoFunction, fdcpy, sizeof(int), (uintptr_t)handler);
}

void WaitFor(void *this)
{
	if (this == NULL)
	{
		perror("WaitFor() failed");
		exit(-1);
	}

	preactor reactor = (preactor)this;

	if (!reactor->hot)
		return;

	fprintf(stdout, "Reactor thread joined.\n");
	pthread_join(reactor->thread, NULL);
}