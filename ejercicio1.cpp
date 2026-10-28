// Ejercicio 1: Singleton
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;
class Account {
private:
    friend class TransactionManager;

    // Operacion atomica: valida y modifica bajo el mismo lock.
    bool apply(double delta) {
        lock_guard<mutex> lock(mutex_);
        if (balance_ + delta < 0) return false;
        balance_ += delta;
        return true;
    }

    string id_;
    double balance_;
    mutable mutex mutex_;
public:
    Account(string id, double initial)
        : id_(move(id)), balance_(initial) {}

    const string& getId() const { return id_; }

    double getBalance() const {
        lock_guard<mutex> lock(mutex_);
        return balance_;
    }


};

class Transaction {
public:
    enum class Type { Deposit, Withdraw };

    Transaction(size_t id, string accountId, Type type,
                double amount, bool success)
        : id_(id), accountId_(move(accountId)), type_(type),
          amount_(amount), success_(success) {}

    size_t getId() const { return id_; }
    const string& getAccountId() const { return accountId_; }
    Type getType() const { return type_; }
    double getAmount() const { return amount_; }
    bool wasSuccessful() const { return success_; }

private:
    size_t id_;
    string accountId_;
    Type type_;
    double amount_;
    bool success_;
};

class TransactionManager {
private:
    TransactionManager() = default;

    void record(Account* account, Transaction::Type type, double amount, bool ok) {
        lock_guard<mutex> lock(logMutex_);
        log_.emplace_back(nextId_++, account->getId(), type, amount, ok);
    }

    mutable mutex logMutex_;
    vector<Transaction> log_;
    size_t nextId_ = 1;
public:
    static TransactionManager& getInstance() {
        static TransactionManager instance;
        return instance;
    }

    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;
    TransactionManager(TransactionManager&&) = delete;
    TransactionManager& operator=(TransactionManager&&) = delete;

    bool deposit(Account* account, double amount) {
        if (!account || amount <= 0) return false;
        bool ok = account->apply(amount);
        record(account, Transaction::Type::Deposit, amount, ok);
        return ok;
    }

    bool withdraw(Account* account, double amount) {
        if (!account || amount <= 0) return false;
        bool ok = account->apply(-amount);
        record(account, Transaction::Type::Withdraw, amount, ok);
        return ok;
    }

    size_t transactionCount() const {
        lock_guard<mutex> lock(logMutex_);
        return log_.size();
    }
    
};

int main() {
    auto& manager = TransactionManager::getInstance();
    Account account("A1", 1000);

    vector<thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            manager.deposit(&account, 100);
            manager.withdraw(&account, 50);
        });
    }
    for (auto& t : threads) t.join();

    cout << "Balance final: " << account.getBalance() << endl;
    cout << "Transacciones registradas: " << manager.transactionCount() << endl;
    cout << "Direccion manager: " << &TransactionManager::getInstance() << endl;
    cout << "Direccion manager: " << &manager << endl;
}
