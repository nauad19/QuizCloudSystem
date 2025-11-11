#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>   // ✅ For Firebase upload

struct Question {
    char question[200];
    char optionA[100], optionB[100], optionC[100], optionD[100];
    char correctOption;
};

// Function declarations
void main_menu();
void admin_menu();
void student_menu();
void add_question();
void view_questions();
void delete_question();
void start_quiz(char name[]);
void save_result(char name[], int score);
void upload_to_firebase(const char *name, int score);  // ✅ New function

// ✅ Firebase Upload Function (with SSL fallback)
void upload_to_firebase(const char *name, int score) {
    CURL *curl;
    CURLcode res;

    const char *url = "https://quizcloudsystem-default-rtdb.asia-southeast1.firebasedatabase.app/results.json";
    char jsonData[200];
    sprintf(jsonData, "{\"name\": \"%s\", \"score\": %d}", name, score);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData);

        // ✅ Set CA certificate path (auto-detect or fallback)
        curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        // ✅ Perform request
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "⚠️ Firebase upload failed: %s\n", curl_easy_strerror(res));

            // 🔁 Try again with SSL verification disabled (last resort)
            printf("⚠️ Retrying without SSL verification...\n");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            res = curl_easy_perform(curl);

            if (res != CURLE_OK)
                fprintf(stderr, "❌ Upload still failed: %s\n", curl_easy_strerror(res));
            else
                printf("✅ Result uploaded to Firebase (unverified SSL)!\n");
        } else {
            printf("✅ Result uploaded to Firebase securely!\n");
        }

        curl_easy_cleanup(curl);
    }
}

// 🧭 MAIN FUNCTION
int main() {
    printf("========================================\n");
    printf("        HYBRID QUIZ TEST SYSTEM         \n");
    printf("========================================\n");

    main_menu();
    return 0;
}

// ===== MAIN MENU =====
void main_menu() {
    int choice;

    while (1) {
        printf("\n===== MAIN MENU =====\n");
        printf("1. Admin Login\n");
        printf("2. Student Login\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                admin_menu();
                break;
            case 2:
                student_menu();
                break;
            case 3:
                printf("Exiting the system... Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
}

// ===== ADMIN MENU =====
void admin_menu() {
    int choice;
    char password[20];

    printf("\nEnter Admin Password: ");
    scanf("%s", password);

    if (strcmp(password, "admin123") != 0) {
        printf("Access Denied! Invalid Password.\n");
        return;
    }

    while (1) {
        printf("\n===== ADMIN MENU =====\n");
        printf("1. Add Question\n");
        printf("2. View All Questions\n");
        printf("3. Delete All Questions\n");
        printf("4. Return to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                add_question();
                break;
            case 2:
                view_questions();
                break;
            case 3:
                delete_question();
                break;
            case 4:
                return;
            default:
                printf("Invalid choice!\n");
        }
    }
}

// ===== ADD QUESTION =====
void add_question() {
    struct Question q;
    FILE *fp = fopen("questions.txt", "a");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    getchar(); // Clear buffer
    printf("\nEnter question: ");
    fgets(q.question, sizeof(q.question), stdin);
    q.question[strcspn(q.question, "\n")] = '\0';

    printf("Enter option A: ");
    fgets(q.optionA, sizeof(q.optionA), stdin);
    q.optionA[strcspn(q.optionA, "\n")] = '\0';

    printf("Enter option B: ");
    fgets(q.optionB, sizeof(q.optionB), stdin);
    q.optionB[strcspn(q.optionB, "\n")] = '\0';

    printf("Enter option C: ");
    fgets(q.optionC, sizeof(q.optionC), stdin);
    q.optionC[strcspn(q.optionC, "\n")] = '\0';

    printf("Enter option D: ");
    fgets(q.optionD, sizeof(q.optionD), stdin);
    q.optionD[strcspn(q.optionD, "\n")] = '\0';

    printf("Enter correct option (A/B/C/D): ");
    scanf(" %c", &q.correctOption);
    q.correctOption = toupper(q.correctOption);

    fprintf(fp, "%s;%s;%s;%s;%s;%c\n",
            q.question, q.optionA, q.optionB, q.optionC, q.optionD, q.correctOption);
    fclose(fp);
    printf("✅ Question added successfully!\n");
}

// ===== VIEW QUESTIONS =====
void view_questions() {
    struct Question q;
    FILE *fp = fopen("questions.txt", "r");

    if (fp == NULL) {
        printf("No questions found!\n");
        return;
    }

    int count = 0;
    while (fscanf(fp, "%[^;];%[^;];%[^;];%[^;];%[^;];%c\n",
                  q.question, q.optionA, q.optionB, q.optionC, q.optionD, &q.correctOption) != EOF) {
        count++;
        printf("\nQ%d: %s\nA) %s\nB) %s\nC) %s\nD) %s\nCorrect: %c\n",
               count, q.question, q.optionA, q.optionB, q.optionC, q.optionD, q.correctOption);
    }

    if (count == 0)
        printf("No questions available!\n");

    fclose(fp);
}

// ===== DELETE ALL QUESTIONS =====
void delete_question() {
    FILE *fp = fopen("questions.txt", "w");
    if (fp == NULL) {
        printf("Error deleting questions!\n");
        return;
    }
    fclose(fp);
    printf("🗑 All questions deleted successfully!\n");
}

// ===== STUDENT MENU =====
void student_menu() {
    int choice;
    char name[50];

    printf("\n===== STUDENT MENU =====\n");
    printf("Enter your name: ");
    scanf("%s", name);

    while (1) {
        printf("\n1. Start Quiz\n2. View Previous Results\n3. Return to Main Menu\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                start_quiz(name);
                break;
            case 2: {
                FILE *f = fopen("results.txt", "r");
                if (f == NULL) {
                    printf("No results found!\n");
                } else {
                    char line[200];
                    printf("\n=== Previous Results ===\n");
                    while (fgets(line, sizeof(line), f))
                        printf("%s", line);
                    fclose(f);
                }
                break;
            }
            case 3:
                return;
            default:
                printf("Invalid choice!\n");
        }
    }
}

// ===== START QUIZ =====
void start_quiz(char name[]) {
    struct Question q;
    FILE *fp = fopen("questions.txt", "r");

    if (fp == NULL) {
        printf("Quiz not available!\n");
        return;
    }

    int score = 0, total = 0;
    char ans;

    while (fscanf(fp, "%[^;];%[^;];%[^;];%[^;];%[^;];%c\n",
                  q.question, q.optionA, q.optionB, q.optionC, q.optionD, &q.correctOption) != EOF) {
        total++;
        printf("\nQ%d. %s\nA) %s\nB) %s\nC) %s\nD) %s\n", total, q.question, q.optionA, q.optionB, q.optionC, q.optionD);
        printf("Enter your answer (A/B/C/D): ");
        scanf(" %c", &ans);
        ans = toupper(ans);

        if (ans == q.correctOption) {
            printf("✅ Correct!\n");
            score++;
        } else {
            printf("❌ Wrong! Correct answer was %c\n", q.correctOption);
        }
    }

    fclose(fp);
    printf("\nYour Score: %d/%d\n", score, total);
    save_result(name, score);
}

// ===== SAVE RESULT LOCALLY + CLOUD UPLOAD =====
void save_result(char name[], int score) {
    FILE *f = fopen("results.txt", "a");
    if (f == NULL) {
        printf("Error saving result!\n");
        return;
    }
    fprintf(f, "Name: %s | Score: %d\n", name, score);
    fclose(f);
    printf("💾 Result saved locally.\n");

    upload_to_firebase(name, score); // ✅ Upload to Firebase
}
