// QuantPractice.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <numbers>
#include <numeric>
#include <functional>
#include <algorithm>
#include <string>
#include <sstream>



//The goal of this notebook is to get familiar with c++ again and utilize it to code a few tools from quant to better familiarize myself with the concepts. 
// Level 0 - Basics
// Print Hello world

void opg0() {
std::cout << "Hello world";
}

// Make mean function and find mean of a random vector

double mean(const std::vector<double>& v) {
    return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}

void opg1() {
std::vector<double> v = { 1,1,4 };
std::cout << mean(v);
//return mean(v);
}

// Utilize prior mean function but with a dynamic list
void opg2() {
    int n;
    std::cout << "How long is the vector you wish to mean?";
    std::cin >> n;

    std::vector<double> choice(n);
    std::cout << "Pick " << n << " numbers in the vector\n";
    for (int i = 0; i < n; ++i) {
        std::cin >> choice[i];
    }
    std::cout << mean(choice);
    std::cout << "\nNext time we use a smart way";
}

// Here we go from A(easy) to E(hardish) task levels
// A1 - Read a comma-separated line of doubles → vector. Print count, mean, min, max, median, double the sorted list and print it.
void A1() {
    std::string Answer;
    std::vector<double> Vec;
    std::cout << "Write me a comma-seperated line of doubles :) \n ";
    std::cin.ignore();
    std::getline(std::cin, Answer);
    std::stringstream ss(Answer);
    std::string temp_val;
    char comma;
    while (std::getline(ss,temp_val,',')) {
        Vec.push_back(std::stod(temp_val));
    }
    int count = Vec.size();
    std::cout << "Count: " << count;
    std::cout << "\nMean: " << mean(Vec);
    double Vmin = Vec[0], Vmax = Vec[0];
    for (int i = 0;i < Vec.size(); i++) {
        if (Vec[i] < Vmin) {
            Vmin = Vec[i];
        } else if (Vec[i] > Vmax) {
            Vmax = Vec[i];
        } else {
        };
    }
    std::cout << "\nMin: " << Vmin;
    std::cout << "\nMax: " << Vmax;
    std::sort(Vec.begin(), Vec.end());
    double median;
    if (Vec.size() % 2 == 1) {
        median = Vec[(Vec.size() - 1) / 2];
    }
    else {
        median = (Vec[(Vec.size() - 1) / 2] + Vec[(Vec.size()) / 2]) / 2;
    };
    std::cout << "\nMedian: " << median;

    std::vector<double> Vec2(Vec.size());
    std::transform(Vec.begin(), Vec.end(), Vec2.begin(), [](double a) {
        return a * 2;
        });
    std::cout << "\nDoubled sorted vector: ";
    for (auto i : Vec2)
        std::cout << i << " ";

}

//B1, make some finance tools (present value helper, zero coupon bond pricer and cashflow Net Present Value)
double pv(double cf, double y, int t) {
    return cf / std::pow(1 + y, t);
}

double zero_coupon_bond(double F, double y, int T) {
    return pv(F,y,T);
}

double cf_NPV(std::vector<double>& CF, double y) {
    double Value = 0;
    for (int i = 0;i < CF.size();i++) {
        Value += pv(CF[i], y, i + 1);
    }
    return Value;
}

void B1() {
    std::cout << "pv test: " << pv(100, 0.05, 2);
    std::cout << "\nZCB test: " << zero_coupon_bond(100, 0.03, 5);
    std::vector<double> test_vector = { 5, 5, 5, 5, 105 };
    std::cout << "\nCf_NPV test:" << cf_NPV(test_vector, 0.03);
}

//C1 Create a Bond class with relevant const methods to help estimate price, risk and curve etc.

struct Bond {
    double face, coupon_rate;
    int maturity;

    Bond(double face, double coupon_rate, int maturity)
        : face(face), coupon_rate(coupon_rate), maturity(maturity) {}

    double price(double y) const{
        double Value = 0;
        for (int i = 1;i <= maturity;i++) {
            Value += pv(coupon_rate * face + face*(i==maturity), y, i);
        }
        return Value;
    }

    double D_mac(double y) const{ //Macaulay
        double Value = 0;
        for (int i = 1;i <= maturity;i++) {
            Value += i*pv(coupon_rate * face + face*(i==maturity), y, i);
        }
        return Value/price(y);
    }

    double D_mod(double y) const {
        return D_mac(y) / (1 + y);
    }

    double DV01(double y) const {
        return D_mod(y)* price(y) * 1e-4;
    }

    double convexity(double y) const {
        double Value = 0;
        double CF = 0;
        for (int i = 1;i <= maturity;i++) {
            CF = (coupon_rate * face + face * (i == maturity));
            Value += (i * CF * (i+1))/std::pow((1+y),i+2);
        }
        return 1 / price(y) * Value;
    }

    double dPdy(double y) const {
        double Value = 0;
        for (int i = 1;i <= maturity;i++) {
            Value += -i*pv(coupon_rate * face + face * (i == maturity), y, i+1);
        }
        return Value;
    }
};

void C1() {
    Bond b(100, 0.05, 5);
    std::cout << "The price is:" << b.price(0.03);
    std::cout << "\nThe D_mac and D_mod is:" << b.D_mac(0.03) << "and " << b.D_mod(0.03);
    std::cout << "\nThe DV01 is:" << b.DV01(0.03);
    std::cout << "\nThe convexity is:" << b.convexity(0.03);
}
//C2 Create a solver that predicts the yield of a bond given a price.

double Yield_estimator(Bond B,double P_market) {
    double y = 0.02;
    double Delta = 1;
    int Counter = 0;
    while ((B.price(y)-P_market)>0.01 && Counter<10) {
        Delta = (B.price(y) - P_market) / B.dPdy(y);
        y = y - Delta;
        std::cout << y <<"\n";
        Counter += 1;
    }
    return y;
}

void Solver_test() { //Estimates the yield from current price and the bond.
    Bond b(100, 0.05, 5);
    double y_est = Yield_estimator(b, 87);
    std::cout << "The proper yield is: " << y_est <<" with price" << b.price(y_est);
}


// D tasks - Stochastics and Monte-Carlo in C++

double st_dev(std::vector<double> v) {
    double Sdev = 0;
    double meanie = mean(v);
    for (int i = 0;i < v.size(); i++) {
        Sdev += std::pow((v[i] - meanie), 2);
    }
    double sigma = std::pow(Sdev / v.size(), 0.5);
    return sigma;
}

std::vector<double> get_norm(int N) {
    std::random_device rd{};
    std::mt19937 gen{ rd() };
    std::normal_distribution<> dist{ 0,1 };
    std::vector<double> items;
    items.reserve(N);
    for (int i = 0;i < N;i++) {
        items.push_back(dist(gen));
    }
    return items;
}

void D1() { //Create randomly distributed points, check the help functions stdev, mean and norm works. 
    std::vector<double> items = get_norm(1e6);
    std::cout << "Mean: " << mean(items) << "    stdev: " << st_dev(items);
    }

void D2() {//Task 9 - Simulate a lot of stocks at strike time T, see what the price C should be.
    int N_paths = 2e5;
    int T = 1;
    double K = 100;
    double S0 = 100, r = 0.03, sigma = 0.2;
    std::vector<double> Stocks;
    std::vector<double> Noise = get_norm(N_paths);
    for (int i = 0;i < N_paths; i++) {
        double S_T = S0 * std::exp((r - 0.5 * sigma * sigma) * T + sigma * std::sqrt(T) * Noise[i]); //C=e^-(rt)*<max(S(T)-K,0)>, so we estimate expected value statistically.
        Stocks.push_back(S_T);
    }
    std::cout << "Mean: " << mean(Stocks)<< " Std: "<< st_dev(Stocks) << " Expectation: " << S0 * std::exp(r * T);
    std::vector<double> Payout(Stocks.size());
    std::transform(Stocks.begin(), Stocks.end(), Payout.begin(), [K](double a) {
        return std::max(a - K, 0.0);
    });
    std::cout << "\nPrice of option C is: " << std::exp(-r * T) * mean(Payout);
}

// Now we are at the final stage, Black-Scholes etc.
double pi = 3.14159265358979323846;

double nphi(double x){
    return std::exp(-(x * x) / 2) / std::sqrt(2 * pi);
}
double phi(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2));
}

double calc_d1(double S0, double K, double r, double sigma, int T) {
    return (std::log(S0 / K) + (r + sigma * sigma / 2) * T) / (sigma * std::sqrt(T));
}
double calc_d2(double S0, double K, double r, double sigma, int T) {
    return (std::log(S0 / K) + (r - sigma * sigma / 2) * T) / (sigma * std::sqrt(T));
}

double bs_call(double S0, double K, double r, double sigma, int T) { //Solution to Black-scholes
    double d1 = calc_d1(S0,K,r,sigma,T);
    double d2 = calc_d2(S0,K,r,sigma,T);
    return S0 * phi(d1) - K * std::exp(-r * T) * phi(d2);
}

double sigma_solver(double C, double S, double K, double r, int T) {//Solves 
    double sig_est = 0.123;
    double Loss = bs_call(S, K, r, sig_est, T) - C;
    double dF = 0, d1=0;
    while (abs(Loss) > 0.00001) {
        d1 = calc_d1(S,K,r,sig_est,T);
        dF = S * std::sqrt(T) * nphi(d1);
        sig_est -= Loss / dF;
        Loss = bs_call(S, K, r, sig_est, T) - C;
    }
    return sig_est;
}

void E1() { //Solution to BS in bs_call, now we find vol given a price C, along other parameters. 
    std::cout << phi(0) << "   " << phi(1);
    std::cout << "\nbs_call: " << bs_call(100, 100, 0.03, 0.2, 1);
    std::cout << "\n Estimated sigma: " << sigma_solver(bs_call(100,100,0.03,0.1412,3), 100, 100, 0.03, 3);
}

int main()
{
    std::vector<std::function<void()>> opgaver = {
        opg0,opg1,opg2,A1,B1,C1, Solver_test,D1,D2,E1
    };
    int choice;
    std::cout << "Choose task: ";
    std::cin >> choice;
    if (choice >= 0 && choice <= opgaver.size()) {
        opgaver[choice]();
    } else {
        std::cout << "Invalid choice \n";
    };
    return 0;
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
