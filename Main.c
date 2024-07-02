#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>

struct FileInfo {
    char name[256];
    unsigned long long int size;
    time_t date;
};

struct TreeNode {
    struct FileInfo data;
    struct TreeNode* left;
    struct TreeNode* right;
};

void inOrderTraversal(struct TreeNode* root);
struct TreeNode* insertByName(struct TreeNode* root, struct FileInfo data);
struct TreeNode* insertBySize(struct TreeNode* root, struct FileInfo data);
struct TreeNode* insertByDate(struct TreeNode* root, struct FileInfo data);

void maxHeapify(struct FileInfo arr[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    // Compare left child with parent
    if (left < n && arr[left].size > arr[largest].size)
        largest = left;

    // Compare right child with largest so far
    if (right < n && arr[right].size > arr[largest].size)
        largest = right;

    // If largest is not the parent, swap and heapify further
    if (largest != i) {
        struct FileInfo temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;
        maxHeapify(arr, n, largest);
    }
}

void heapSort(struct FileInfo arr[], int n) {
    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--)
        maxHeapify(arr, n, i);

    // Extract elements one by one
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        struct FileInfo temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;

        // Call maxHeapify on the reduced heap
        maxHeapify(arr, i, 0);
    }
}

void readAndSortFiles(const char* dirPath, struct TreeNode* (*insertFunction)(struct TreeNode*, struct FileInfo)) {
    struct dirent* entry;
    DIR* dp = opendir(dirPath);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    int fileCount = 0;
    struct FileInfo* fileArray = malloc(sizeof(struct FileInfo));

    while ((entry = readdir(dp)) != NULL) {
        struct stat fileStat;
        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

        if (stat(filePath, &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            fileCount++;
            fileArray = realloc(fileArray, fileCount * sizeof(struct FileInfo));

            struct FileInfo fileInfo;
            strncpy(fileInfo.name, entry->d_name, sizeof(fileInfo.name));
            fileInfo.size = fileStat.st_size;
            fileInfo.date = fileStat.st_mtime;

            fileArray[fileCount - 1] = fileInfo;
        }
    }

    closedir(dp);

    // Sort the file array using heap sort by size
    heapSort(fileArray, fileCount);

    // Insert sorted files into the binary search tree
    struct TreeNode* root = NULL;
    for (int i = 0; i < fileCount; i++)
        root = insertFunction(root, fileArray[i]);

    printf("\nSorted Files:\n");
    inOrderTraversal(root);

    // Free dynamically allocated memory
    free(fileArray);
}

char* formatTime(time_t timestamp) {
    char* formattedTime = ctime(&timestamp);
    if (formattedTime[strlen(formattedTime) - 1] == '\n') {
        formattedTime[strlen(formattedTime) - 1] = '\0';
    }
    return formattedTime;
}

void inOrderTraversal(struct TreeNode* root) {
    if (root != NULL) {
        inOrderTraversal(root->left);
        printf("Name: %s, Size: %llu, Date: %s\n\n", root->data.name, root->data.size, formatTime(root->data.date));
        inOrderTraversal(root->right);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <directory_path> <sort_criteria>\n", argv[0]);
        printf("Sort criteria:\n");
        printf("1 - Sort by name\n");
        printf("2 - Sort by size\n");
        printf("3 - Sort by date\n");
        return 1;
    }

    char* dirPath = argv[1];
    int sortCriteria = atoi(argv[2]);

    if (sortCriteria < 1 || sortCriteria > 3) {
        printf("Invalid sort criteria. Please enter a number between 1 and 3.\n");
        return 1;
    }

    struct TreeNode* (*insertFunction)(struct TreeNode*, struct FileInfo);
    switch (sortCriteria) {
        case 1:
            insertFunction = insertByName;
            break;
        case 2:
            insertFunction = insertBySize;
            break;
        case 3:
            insertFunction = insertByDate;
            break;
    }

    readAndSortFiles(dirPath, insertFunction);

    return 0;
}

struct TreeNode* insertByName(struct TreeNode* root, struct FileInfo data) {
    if (root == NULL) {
        struct TreeNode* newNode = (struct TreeNode*)malloc(sizeof(struct TreeNode));
        newNode->data = data;
        newNode->left = newNode->right = NULL;
        return newNode;
    }

    int compareResult = strcmp(data.name, root->data.name);

    if (compareResult < 0)
        root->left = insertByName(root->left, data);
    else
        root->right = insertByName(root->right, data);

    return root;
}

struct TreeNode* insertByDate(struct TreeNode* root, struct FileInfo data) {
    if (root == NULL) {
        struct TreeNode* newNode = (struct TreeNode*)malloc(sizeof(struct TreeNode));
        newNode->data = data;
        newNode->left = newNode->right = NULL;
        return newNode;
    }

    if (data.date < root->data.date)
        root->left = insertByDate(root->left, data);
    else
        root->right = insertByDate(root->right, data);

    return root;
}

struct TreeNode* insertBySize(struct TreeNode* root, struct FileInfo data) {
    if (root == NULL) {
        struct TreeNode* newNode = (struct TreeNode*)malloc(sizeof(struct TreeNode));
        newNode->data = data;
        newNode->left = newNode->right = NULL;
        return newNode;
    }

    if (data.size < root->data.size)
        root->left = insertBySize(root->left, data);
    else
        root->right = insertBySize(root->right, data);

    return root;
}
