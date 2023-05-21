#include "Reactor.h"

// Function to create a new reactor
void *createReactor()
{
	preactor react = NULL;

	if ((react = (preactor)malloc(sizeof(reactor))) == NULL)
	{
		perror("malloc() failed");
		exit(-1);
	}

	// Initialize reactor properties
	react->hot = false;
	react->clients_counter = 0;
	react->size = 4;
	react->thread = 0;
	react->pfds = (struct pollfd *)malloc(sizeof(struct pollfd) * 4);
	if (!react->pfds)
	{
		perror("malloc pfds() failed");
		free(react);
		exit(-1);
	}

	// Create a hashmap to store file descriptors and their corresponding handlers
	react->FDtoFunction = hashmap_create();
	if (!react->FDtoFunction)
	{
		free(react->pfds);
		free(react);
		printf("hashmap_create() failed\n");
		exit(-1);
	}

	fprintf(stdout, "Reactor created.\n");
	return react;
}

// Function to run the reactor in a separate thread
void *reactorRun(void *thisReactor)
{
	if (thisReactor == NULL)
	{
		perror("reactorRun() failed");
		exit(-1);
	}

	preactor reactor = (preactor)thisReactor;

	while (reactor->hot)
	{
		// Perform polling on file descriptors
		if (poll(reactor->pfds, reactor->clients_counter, 1000) == -1)
		{
			perror("poll() failed");
			exit(-1);
		}

		// Process events on file descriptors
		for (size_t i = 0; i < reactor->clients_counter; i++)
		{
			if (reactor->pfds[i].revents & POLLIN)
			{
				// Retrieve the handler associated with the file descriptor
				uintptr_t function;
				if (!hashmap_get(reactor->FDtoFunction, &reactor->pfds[i].fd, sizeof(int), &function))
				{
					printf("hashmap_get() failed\n");
					continue;
				}

				// Call the handler function
				handler_t handler = (handler_t)function;
				handler(reactor->pfds[i].fd, reactor);
			}
		}
	}

	fprintf(stdout, "Reactor thread finished.\n");

	return reactor;
}

// Function to start the reactor
void startReactor(void *thisReactor)
{
	if (thisReactor == NULL)
	{
		perror("startReactor() failed");
		exit(-1);
	}

	preactor react = (preactor)thisReactor;

	if (react->hot)
		return;
	else
	{
		// Set the reactor as active and create a new thread for running the reactor
		react->hot = true;
		if (pthread_create(&react->thread, NULL, reactorRun, thisReactor) != 0)
		{
			perror("pthread_create() failed");
			exit(-1);
		}
		fprintf(stdout, "Reactor thread started.\n");
	}
}

// Function to stop the reactor
void stopReactor(void *thisReactor)
{
	if (thisReactor == NULL)
	{
		perror("stopReactor() failed");
		exit(-1);
	}

	preactor reactor = (preactor)thisReactor;
	if (!reactor->hot)
		return;

	// Stop the reactor thread and wait for it to finish
	reactor->hot = false;
	pthread_cancel(reactor->thread);
	pthread_join(reactor->thread, NULL);

	fprintf(stdout, "Reactor thread stopped and detached.\n");
}

// Function to add a file descriptor and its corresponding handler to the reactor
void addFd(void *thisReactor, int fd, handler_t handler)
{
	if (thisReactor == NULL)
	{
		perror("addFd() failed");
		exit(-1);
	}

	preactor reactor = (preactor)thisReactor;
	int i = reactor->clients_counter;
	for (size_t j = 0; j < reactor->size - 1; j++)
	{
		if (reactor->pfds[j].fd == -1)
		{
			i = j;
		}
	}

	if (i == reactor->size)
	{
		// Resize the pfds array if it is full
		reactor->size *= 2;
		reactor->pfds = (struct pollfd *)realloc(reactor->pfds, sizeof(struct pollfd) * reactor->size);
		if (!reactor->pfds)
		{
			perror("realloc() failed");
			free(reactor);
			return;
		}
	}

	// Add the new file descriptor and its events to the pfds array
	reactor->pfds[i].fd = fd;
	reactor->pfds[i].events = POLLIN;
	reactor->clients_counter++;
	// Create a copy of the file descriptor to store in the hashmap
	int *fdcpy = (int *)malloc(sizeof(int));
	*fdcpy = fd;
	// Add the file descriptor and its handler to the hashmap
	hashmap_set(reactor->FDtoFunction, fdcpy, sizeof(int), (uintptr_t)handler);

	fprintf(stdout, "addFd() success.\n");
}

// Function to wait for the reactor thread to finish
void WaitFor(void *thisReactor)
{
	if (thisReactor == NULL)
	{
		perror("WaitFor() failed");
		exit(-1);
	}

	preactor reactor = (preactor)thisReactor;

	if (!reactor->hot)
		return;

	fprintf(stdout, "Reactor thread joined.\n");
	pthread_join(reactor->thread, NULL);
}
