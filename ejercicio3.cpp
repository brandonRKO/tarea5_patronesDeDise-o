// Ejercicio 3
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
using namespace std;

class Expression;
using ExprPtr = shared_ptr<Expression>;

class Expression {
public:
    virtual ~Expression() = default;
    virtual double evaluate(double x) = 0;
    virtual ExprPtr derivative() = 0;
    virtual string toString() = 0;
    
    virtual optional<double> constantValue() const { return nullopt; }
};


class Constant : public Expression {
public:
    explicit Constant(double v) : value_(v) {}
    double evaluate(double) override { return value_; }
    ExprPtr derivative() override { return make_shared<Constant>(0); }
    string toString() override {
        ostringstream os; os << value_; return os.str();
    }
    optional<double> constantValue() const override { return value_; }
private:
    double value_;
};

class Variable : public Expression {
public:
    double evaluate(double x) override { return x; }
    ExprPtr derivative() override { return make_shared<Constant>(1); }
    string toString() override { return "x"; }
};


class BinaryOperation : public Expression {
public:
    BinaryOperation(ExprPtr l, ExprPtr r) : left_(move(l)), right_(move(r)) {}
    string toString() override {
        return "(" + left_->toString() + " " + symbol() + " " + right_->toString() + ")";
    }
protected:
    virtual string symbol() const = 0;
    ExprPtr left_, right_;
};

class UnaryFunction : public Expression {
public:
    explicit UnaryFunction(ExprPtr arg) : arg_(move(arg)) {}
    string toString() override { return name() + "(" + arg_->toString() + ")"; }
protected:
    virtual string name() const = 0;
    ExprPtr arg_;
};

ExprPtr makeConst(double v);
ExprPtr makeAdd(ExprPtr a, ExprPtr b);
ExprPtr makeSub(ExprPtr a, ExprPtr b);
ExprPtr makeMul(ExprPtr a, ExprPtr b);
ExprPtr makeDiv(ExprPtr a, ExprPtr b);
ExprPtr makePow(ExprPtr a, ExprPtr b);
ExprPtr makeSin(ExprPtr a);
ExprPtr makeCos(ExprPtr a);
ExprPtr makeExp(ExprPtr a);
ExprPtr makeLn(ExprPtr a);

// ---------- Operaciones binarias ----------
class Add : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) override { return left_->evaluate(x) + right_->evaluate(x); }
    // (u + v)' = u' + v'
    ExprPtr derivative() override { return makeAdd(left_->derivative(), right_->derivative()); }
protected:
    string symbol() const override { return "+"; }
};

class Subtract : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) override { return left_->evaluate(x) - right_->evaluate(x); }
    // (u - v)' = u' - v'
    ExprPtr derivative() override { return makeSub(left_->derivative(), right_->derivative()); }
protected:
    string symbol() const override { return "-"; }
};

class Multiply : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) override { return left_->evaluate(x) * right_->evaluate(x); }
    // (u * v)' = u' * v + u * v'
    ExprPtr derivative() override {
        return makeAdd(makeMul(left_->derivative(), right_),
                       makeMul(left_, right_->derivative()));
    }
protected:
    string symbol() const override { return "*"; }
};

class Divide : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) override { return left_->evaluate(x) / right_->evaluate(x); }
    // (u / v)' = (u' * v - u * v') / v^2
    ExprPtr derivative() override {
        return makeDiv(makeSub(makeMul(left_->derivative(), right_),
                               makeMul(left_, right_->derivative())),
                       makePow(right_, makeConst(2)));
    }
protected:
    string symbol() const override { return "/"; }
};

class Power : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) override { return pow(left_->evaluate(x), right_->evaluate(x)); }
    ExprPtr derivative() override {
        auto n = right_->constantValue();
        if (n) {
            // (u^n)' = n * u^(n-1) * u'   (regla de la potencia + cadena)
            return makeMul(makeMul(makeConst(*n), makePow(left_, makeConst(*n - 1))),
                           left_->derivative());
        }
        // (u^v)' = u^v * (v' * ln(u) + v * u' / u)   (caso general)
        return makeMul(
            make_shared<Power>(left_, right_),
            makeAdd(makeMul(right_->derivative(), makeLn(left_)),
                    makeDiv(makeMul(right_, left_->derivative()), left_)));
    }
protected:
    string symbol() const override { return "^"; }
};

// ---------- Funciones unarias ----------
class Sin : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) override { return sin(arg_->evaluate(x)); }
    // sin(u)' = cos(u) * u'
    ExprPtr derivative() override { return makeMul(makeCos(arg_), arg_->derivative()); }
protected:
    string name() const override { return "sin"; }
};

class Cos : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) override { return cos(arg_->evaluate(x)); }
    // cos(u)' = -sin(u) * u'
    ExprPtr derivative() override {
        return makeMul(makeMul(makeConst(-1), makeSin(arg_)), arg_->derivative());
    }
protected:
    string name() const override { return "cos"; }
};

class Exp : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) override { return exp(arg_->evaluate(x)); }
    // exp(u)' = exp(u) * u'
    ExprPtr derivative() override { return makeMul(makeExp(arg_), arg_->derivative()); }
protected:
    string name() const override { return "exp"; }
};

class Ln : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) override { return log(arg_->evaluate(x)); }
    // ln(u)' = u' / u
    ExprPtr derivative() override { return makeDiv(arg_->derivative(), arg_); }
protected:
    string name() const override { return "ln"; }
};

// ---------- Implementacion de los constructores simplificadores ----------
ExprPtr makeConst(double v) { return make_shared<Constant>(v); }

ExprPtr makeAdd(ExprPtr a, ExprPtr b) {
    auto ca = a->constantValue(), cb = b->constantValue();
    if (ca && cb) return makeConst(*ca + *cb);
    if (ca && *ca == 0) return b;
    if (cb && *cb == 0) return a;
    return make_shared<Add>(a, b);
}
ExprPtr makeSub(ExprPtr a, ExprPtr b) {
    auto ca = a->constantValue(), cb = b->constantValue();
    if (ca && cb) return makeConst(*ca - *cb);
    if (cb && *cb == 0) return a;
    if (ca && *ca == 0) return makeMul(makeConst(-1), b);
    return make_shared<Subtract>(a, b);
}
ExprPtr makeMul(ExprPtr a, ExprPtr b) {
    auto ca = a->constantValue(), cb = b->constantValue();
    if (ca && cb) return makeConst(*ca * *cb);
    if ((ca && *ca == 0) || (cb && *cb == 0)) return makeConst(0);
    if (ca && *ca == 1) return b;
    if (cb && *cb == 1) return a;
    return make_shared<Multiply>(a, b);
}
ExprPtr makeDiv(ExprPtr a, ExprPtr b) {
    auto ca = a->constantValue(), cb = b->constantValue();
    if (ca && *ca == 0) return makeConst(0);
    if (cb && *cb == 1) return a;
    if (ca && cb && *cb != 0) return makeConst(*ca / *cb);
    return make_shared<Divide>(a, b);
}
ExprPtr makePow(ExprPtr a, ExprPtr b) {
    auto cb = b->constantValue();
    if (cb && *cb == 1) return a;
    if (cb && *cb == 0) return makeConst(1);
    return make_shared<Power>(a, b);
}
ExprPtr makeSin(ExprPtr a) { return make_shared<Sin>(a); }
ExprPtr makeCos(ExprPtr a) { return make_shared<Cos>(a); }
ExprPtr makeExp(ExprPtr a) { return make_shared<Exp>(a); }
ExprPtr makeLn(ExprPtr a) { return make_shared<Ln>(a); }

int main() {
    auto x = make_shared<Variable>();
    auto expr =
        make_shared<Multiply>(
            make_shared<Add>(
                make_shared<Power>(
                    x,
                    make_shared<Constant>(2)
                ),
                make_shared<Multiply>(
                    make_shared<Constant>(3),
                    x
                )
            ),
            make_shared<Sin>(x)
        );

    auto deriv = expr->derivative();

    cout << "f(x) = " << expr->toString() << endl;
    cout << "f'(x) = " << deriv->toString() << endl;
    cout << "f(2) = " << expr->evaluate(2) << endl;
    cout << "f'(2) = " << deriv->evaluate(2) << endl;

    // Anidamiento arbitrario: g(x) = ln(exp(x^2) / x)
    auto g = make_shared<Ln>(
        make_shared<Divide>(
            make_shared<Exp>(make_shared<Power>(x, make_shared<Constant>(2))),
            x));
    cout << "\ng(x) = " << g->toString() << endl;
    cout << "g'(x) = " << g->derivative()->toString() << endl;
    cout << "g'(2) = " << g->derivative()->evaluate(2) << endl;
}
