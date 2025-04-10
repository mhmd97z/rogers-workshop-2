#!/bin/bash


# Define the questions and answers
questions=(
    "Which of the following is NOT a key objective of O-RAN?\n1) Promoting openness and interoperability in RAN components\n2) Enabling real-time AI/ML-driven RAN optimization\n3) Strengthening vendor lock-in by requiring proprietary hardware\n4) Reducing operational costs through disaggregation"
    "What is the role of the O-RAN Alliance?\n1) To define open standards and specifications for disaggregated RAN components\n2) To regulate spectrum allocation for wireless networks\n3) To develop proprietary hardware for telecom operators\n4) To replace 3GPP as the primary standards body for 5G"
    "Which of the following best describes an xApp in the O-RAN framework?\n1) A cloud-native application deployed on the Non-RT RIC to optimize RAN configurations\n2) A software component running on the Near-RT RIC that enables closed-loop control and RAN optimization\n3) A protocol used for E2 interface communication\n4) A function within the 5G core network for mobility management"
    "Which of the following best describes the interaction between xApps and the Near-RT RIC?\n1) xApps are independent of the Near-RT RIC and operate directly with the RAN nodes\n2) xApps interact with the Near-RT RIC via internal APIs to access E2 data and issue control commands\n3) xApps require the SMO to establish a connection with the Near-RT RIC\n4) xApps are deployed directly on the Non-RT RIC and do not interact with the Near-RT RIC"
    "Which of the following is NOT an example of an xApp use case?\n1) Energy-efficient power control in the RAN\n2) Dynamic spectrum sharing\n3) UE authentication and security policy enforcement\n4) Load balancing across multiple cells"
    "What is a key challenge when designing xApps for the Near-RT RIC?\n1) Ensuring they can operate independently of the O-RAN architecture\n2) Guaranteeing compatibility with proprietary RAN solutions\n3) Managing real-time data processing and low-latency decision-making\n4) Avoiding communication with E2 nodes"
    "What is the purpose of the E2 interface in the O-RAN architecture?\n1) Connecting the Near-RT RIC with the O-RAN Service Management and Orchestration (SMO)\n2) Enabling communication between the Near-RT RIC and the O-CU/O-DU\n3) Providing a connection between the RAN and the 5GC\n4) Serving as an interface between xApps and the Near-RT RIC"
    "What is the function of the E2 Application Protocol (E2AP)?\n1) It enables real-time user authentication in the RAN\n2) It facilitates communication between xApps and the Non-RT RIC\n3) It provides signaling procedures for control and monitoring between the Near-RT RIC and E2 nodes\n4) It acts as a transport protocol for user data between the RAN and the 5GC"
    "Which of the following is an example of an E2AP procedure?\n1) E2 Node Setup\n2) User Equipment Handover Request\n3) PDU Session Establishment\n4) NAS Security Mode Command"
    "What is the purpose of the E2 Service Model (E2SM) in the O-RAN framework?\n1) To define the structure of Non-RT RIC applications\n2) To specify the data and control exchange between the Near-RT RIC and E2 nodes\n3) To describe xApp configurations in the Near-RT RIC\n4) To provide a graphical user interface for the O-RAN SMO"
)

answers=(3 1 2 2 3 3 2 3 1 2)

# Track score
score=0

echo "Welcome to the Quiz 1 on O-RAN!"
echo "This quiz consists of 10 multiple-choice questions."
echo "Answer each question by typing the number corresponding to your choice."

i=0
for question in "${questions[@]}"; do
    echo -e "\nQuestion $((i+1)):"
    echo -e "$question"
    
    while true; do
        read -p "Your answer (1-4): " user_answer
        
        if [[ "$user_answer" =~ ^[1-4]$ ]]; then
            break
        else
            echo "Invalid input. Please enter a number between 1 and 4."
        fi
    done
    
    if [[ "$user_answer" -eq "${answers[i]}" ]]; then
        echo "Correct!"
        ((score++))
    else
        echo "Incorrect. The correct answer was ${answers[i]}"
    fi
    
    ((i++))
done

echo -e "\nQuiz complete! Your final score: $score/${#questions[@]}"

