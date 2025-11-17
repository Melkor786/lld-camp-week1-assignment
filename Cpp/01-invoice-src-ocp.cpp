// 01-invoice-srp-ocp.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
using namespace std;

struct LineItem
{
    string sku;
    int quantity{0};
    double unitPrice{0.0};
};



// ------------------ Discount Strategy -------------------------
class IDiscountStrategy
{
public:
    virtual double compute(double subtotal) const = 0;
    virtual ~IDiscountStrategy() = default;
};

class PercentOff : public IDiscountStrategy
{
    double percent;

public:
    PercentOff(double p) : percent(p) {}
    double compute(double subtotal) const override
    {
        return subtotal * (percent / 100);
    }
};

class FlatOff : public IDiscountStrategy
{
    int amount;

public:
    FlatOff(int a) : amount(a) {}
    double compute(double) const override
    {
        return amount;
    }
};




// ------------------ Tax Strategy -------------------------
class ITaxRule
{
public:
    virtual double compute(double base) const = 0;
    virtual ~ITaxRule() = default;
};

class GST18 : public ITaxRule
{
public:
    double compute(double base) const override
    {
        return base * 0.18;
    }
};



// ------------------ Rendering Strategy -------------------------
class IInvoiceRenderer
{
    public:
    virtual string render(const vector<LineItem> &items,
                          double subtotal,
                          double discounts,
                          double tax,
                          double total) const = 0;
    virtual ~IInvoiceRenderer() = default;
};

class SimpleTextRenderer : public IInvoiceRenderer
{
public:
    string render(const vector<LineItem> &items,
                  double subtotal,
                  double discount,
                  double tax,
                  double grand) const override
    {
        ostringstream out;
        out << "INVOICE\n";
        for (auto &it : items)
            out << it.sku << " x" << it.quantity << " @ " << it.unitPrice << "\n";

        out << "Subtotal: " << subtotal << "\n";
        out << "Discounts: " << discount << "\n";
        out << "Tax: " << tax << "\n";
        out << "Total: " << grand << "\n";
        return out.str();
    }
};



// ------------------ Email Service -------------------------
class IEmailService
{
public:
    virtual void send(const string &email, const string &content) = 0;
    virtual ~IEmailService() = default;
};

class ConsoleEmailService : public IEmailService
{
public:
    void send(const string &email, const string &) override
    {
        cout << "[SMTP] Sending invoice to " << email << "...\n";
    }
};



// ------------------ Logger -------------------------
class ILogger
{
public:
    virtual void log(const string &msg) = 0;
    virtual ~ILogger() = default;
};

class ConsoleLogger : public ILogger
{
public:
    void log(const string &msg) override
    {
        cout << "[LOG] " << msg << "\n";
    }
};




// -------------------InvoiceService Class ----------------------
class InvoiceService
{
    unique_ptr<ITaxRule> taxRule;
    unique_ptr<IInvoiceRenderer> renderer;
    unique_ptr<IEmailService> emailer;
    unique_ptr<ILogger> logger;

public:
    InvoiceService(unique_ptr<ITaxRule> t,
                   unique_ptr<IInvoiceRenderer> r,
                   unique_ptr<IEmailService> e,
                   unique_ptr<ILogger> l)
        : taxRule(move(t)), renderer(move(r)), emailer(move(e)), logger(move(l)) {}

    string process(const vector<LineItem> &items,
                   const vector<unique_ptr<IDiscountStrategy>> &discounts,
                   const string &email)
    {
        double subtotal = 0.0;
        for (auto &it : items)
            subtotal += it.unitPrice * it.quantity;

        double discount_total = 0.0;
        for (auto &d : discounts)
            discount_total += d->compute(subtotal);

        double tax = taxRule->compute(subtotal - discount_total);
        double grand = subtotal - discount_total + tax;

        string content = renderer->render(items, subtotal, discount_total, tax, grand);

        if (!email.empty())
            emailer->send(email, content);
        logger->log("Invoice processed for " + email + " total=" + to_string(grand));

        return content;
    }
};

int main()
{
    vector<LineItem> items = {
        {"ITEM-001", 3, 100.0},
        {"ITEM-002", 1, 250.0}};

    vector<unique_ptr<IDiscountStrategy>> discounts;
    discounts.push_back(make_unique<PercentOff>(10.0));

    InvoiceService svc(
        make_unique<GST18>(),
        make_unique<SimpleTextRenderer>(),
        make_unique<ConsoleEmailService>(),
        make_unique<ConsoleLogger>());

    cout << svc.process(items, discounts, "customer@example.com");

    return 0;
}
