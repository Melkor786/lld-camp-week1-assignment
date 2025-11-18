// 03-notify-dip-ocp.cpp
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// ------------------------ Low-level Services ------------------------

class IEmailService {
public:
    virtual void sendEmail(const string& templ,
                           const string& to,
                           const string& body) = 0;
    virtual ~IEmailService() = default;
};

class ISmsService {
public:
    virtual void sendSMS(const string& phone,
                         const string& message) = 0;
    virtual ~ISmsService() = default;
};

class SmtpMailer : public IEmailService {
public:
    void sendEmail(const string& templ,
                   const string& to,
                   const string& body) override {
        cout << "[SMTP] template=" << templ
             << " to=" << to
             << " body=" << body << "\n";
    }
};

class TwilioClient : public ISmsService {
public:
    void sendSMS(const string& phone,
                 const string& message) override {
        cout << "[Twilio] OTP " << message
             << " -> " << phone << "\n";
    }
};

// ------------------------ User Model ------------------------

struct User {
    string email;
    string phone;
    User(const string& e, const string& p)
        : email(e), phone(p) {}
};

// ------------------------ Notification Abstraction ------------------------

class INotifier {
public:
    virtual void notify(const User& u) = 0;
    virtual ~INotifier() = default;
};

// ------------------------ Concrete Notifiers ------------------------

class WelcomeEmailNotifier : public INotifier {
private:
    IEmailService* email;
public:
    WelcomeEmailNotifier(IEmailService* svc) : email(svc) {}

    void notify(const User& u) override {
        email->sendEmail("welcome", u.email, "Welcome!");
    }
};

class OTPNotifier : public INotifier {
private:
    ISmsService* sms;
public:
    OTPNotifier(ISmsService* svc) : sms(svc) {}

    void notify(const User& u) override {
        sms->sendSMS(u.phone, "123456");
    }
};

// ------------------------ Composite Notifier (OCP) ------------------------

class CompositeNotifier : public INotifier {
private:
    vector<INotifier*> notifiers;
public:
    void add(INotifier* notifier) {
        notifiers.push_back(notifier);
    }

    void notify(const User& u) override {
        for (auto n : notifiers)
            n->notify(u);
    }
};

// ------------------------ High-level SignUp Service ------------------------

class SignUpService {
private:
    INotifier* notifier;  // ONLY ONE DEPENDENCY (DIP)
public:
    SignUpService(INotifier* n) : notifier(n) {}

    bool signUp(const User& u) {
        if (u.email.empty())
            return false;

        // Imagine DB save logic here...

        notifier->notify(u);  // triggers all notifications
        return true;
    }
};

// ------------------------ MAIN: Composition Root ------------------------

int main() {
    // Concrete dependencies
    SmtpMailer smtp;
    TwilioClient twilio;

    // Individual notifiers
    WelcomeEmailNotifier welcomeEmail(&smtp);
    OTPNotifier otp(&twilio);

    // Composite notifier
    CompositeNotifier composite;
    composite.add(&welcomeEmail);
    composite.add(&otp);

    // High level service depends ONLY on abstraction
    SignUpService svc(&composite);

    User user("user@example.com", "+15550001111");
    svc.signUp(user);

    return 0;
}
