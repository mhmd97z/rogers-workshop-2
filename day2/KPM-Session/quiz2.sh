#!/bin/bash

# Define the questions and answers
questions=(
    "What is the primary purpose of the KPM E2 Service Model (E2SM)?\n1) To facilitate AI/ML-driven RAN control in the Near-RT RIC\n2) To enable the monitoring and reporting of key performance indicators (KPIs) from E2 nodes\n3) To provide security authentication for xApps in the Near-RT RIC\n4) To establish a direct communication channel between the Non-RT RIC and E2 nodes"
    "Which type of data does the KPM E2SM typically collect from an E2 node?\n1) UE authentication logs and encryption keys\n2) RSRP, SINR, throughput, and packet loss metrics\n3) Core network session establishment data\n4) Handshake messages between the O-RU and O-DU"
    "In the O-RAN framework, which component subscribes to KPM data using the E2SM-KPM?\n1) The SMO\n2) The xApp running on the Near-RT RIC\n3) The Non-RT RIC policy engine\n4) The O-RU"
    "What is the main difference between the periodic and event-triggered reporting modes in E2SM-KPM?\n1) Periodic reporting occurs at fixed time intervals, whereas event-triggered reporting occurs when predefined thresholds are crossed\n2) Event-triggered reporting collects data continuously, while periodic reporting is only for alarms\n3) Periodic reporting is initiated by the xApp, while event-triggered reporting is initiated by the E2 node\n4) Event-triggered reporting only applies to RAN control actions, not performance monitoring"
    "In the E2SM-KPM specification, which of the following elements defines the format and content of performance measurement reports?\n1) E2 Node Configuration\n2) KPM Measurement Types\n3) KPM Report Style\n4) KPM Indication Header"
    "What role does the E2 Subscription procedure play in KPM E2SM operation?\n1) It allows the xApp to request specific KPIs from E2 nodes\n2) It triggers the AI/ML model deployment process\n3) It is used for establishing an xApp-to-xApp communication channel\n4) It configures the Non-RT RIC with E2 node parameters"
    "In E2SM-KPM, what is a Measurement Information element?\n1) A descriptor that defines which KPIs are being reported in an E2 message\n2) A unique identifier assigned to each E2 node\n3) A list of available AI/ML models in the Near-RT RIC\n4) A policy rule that determines access control in the E2 interface"
    "What is the purpose of the Action Definition field in an E2SM-KPM message?\n1) It specifies the type of measurement action requested, such as periodic or event-triggered reporting\n2) It defines the authentication mechanism for accessing KPI data\n3) It controls the beamforming strategy in O-RUs\n4) It manages RAN slicing configurations at the core network level"
    "How can the KPM xApp improve network performance based on E2SM-KPM reports?\n1) By recommending adaptive parameter tuning for RAN components based on KPI trends\n2) By disabling event-triggered reporting to reduce network overhead\n3) By modifying E2AP messages directly to enhance spectral efficiency\n4) By limiting AI-based optimizations to avoid conflicts with the Non-RT RIC"
    "Which of the following is a potential challenge when using E2SM-KPM in O-RAN networks?\n1) Ensuring efficient processing of high-frequency KPI reports without overloading the Near-RT RIC\n2) Preventing the E2SM-KPM from influencing RAN optimization decisions\n3) Eliminating the need for AI/ML-based performance monitoring\n4) Restricting KPI access to only proprietary vendors"
)

# Define correct answers (corresponding choice numbers)
answers=(2 2 2 1 3 1 1 1 1 1)

# Track score
score=0

echo "Welcome to the Quiz 2 on KPM E2SM!"
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

