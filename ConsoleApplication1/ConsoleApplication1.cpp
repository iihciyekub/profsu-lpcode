/*
Cees Dert, Bart Oldenkamp, (2000) Optimal Guaranteed Return Portfolios and the Casino Effect.
Operations Research 48(5) : 768 - 775. http ://dx.doi.org/10.1287/opre.48.5.768.12400

GitHub : https://github.com/iihciyekub/profsu-lpcode

Author: Li,YougJian
March 19,2021 9:30
IDE : Microsoft Visual Studio Professional 2013
ILOG CPLEX Optimization Studio 12.6.1
*/

#include <iostream>
#include <stdlib.h>
#include<ilcplex/ilocplex.h>
ILOSTLBEGIN


int main()
{
#pragma region(S) CPLEX base set
	IloEnv env;

	// CPLEX syntax, create objective function
	IloModel model(env);

	// num of call or put option 
	int num = 7;
	// Buy CALL or PUT option (see,P769) '(positive) Amount of put and call options in the portfolio'
	IloNumVarArray call_x_Ask(env), put_x_Ask(env);
	// Sell CALL or PUT option (see,P769) '(negative) Amount of put and call options in the portfolio'
	IloNumVarArray call_x_Bid(env), put_x_Bid(env);

	// CPLEX syntax, create variables
	for (size_t i = 0; i < num; i++)
	{
		call_x_Ask.add(IloNumVar(env, 0, IloInfinity, ILOFLOAT));
		put_x_Ask.add(IloNumVar(env, 0, IloInfinity, ILOFLOAT));
		call_x_Bid.add(IloNumVar(env, -IloInfinity, 0, ILOFLOAT));
		put_x_Bid.add(IloNumVar(env, -IloInfinity, 0, ILOFLOAT));

	}
	// CPLEX syntax: create  variables. (see,P769) 'number of units of the index in the portfolio'
	IloNumVar y(env, 0, IloInfinity, ILOFLOAT);

	// CPLEX syntax: create  variables. (see,P769) 'Amount of money invested in the risk-free
	IloNumVar z(env, 0, IloInfinity, ILOFLOAT);


	// (see.p769) The exercise price of the ith option, 
	IloIntArray k(env, num);

	// (see.p769) Prices of European put and call options with exercise prices Ki, respectively, expiringat the horizon T
	IloNumArray call_Bid(env, num), call_Ask(env, num), call_Expected(env, num);
	IloNumArray put_Bid(env, num), put_Ask(env, num), put_Expected(env, num);

	// S&P500 
	IloNum S_0 = 760.48;
	IloNum S_T = 765.32;

	IloNum cash = 100;
	IloNum theta = 0.9;
	IloNum r = 0.0037;

	IloNum rate_by_year = 0.0037;

	IloNum SD_by_year = 0.179;
	// see.p771 "The present value of estimated dividends is $1.59"
	IloNum Dividends = 1.59;

	IloNum horizon = 23;
	// this paper sets the total 360 days in year
	IloNum Day_of_year = 360;

	// (see.p771) "with an expected annualized growth rate of 10%". 760.48*(1+0.1*23/360)=765.338622222222
	IloNum mu_by_year = 765.338622222222;

#pragma endregion



#pragma region(S) load data

	FILE* input_para;
	char tempstr[256];
	double tempnum;
	auto filepath = "./data.txt";
	while (fopen_s(&input_para, filepath, "r") != 0)
	{
		cout << "Didn't find 'data.txt' file in the current directory  ... ...\nAfter placing the file, press Enter" << endl;
		system("pause");
	}

	printf("\tBid\tAks\tExp.turn\n", r);


	// skip the first raw
	fgets(tempstr, 255, input_para);
	// read call option data
	for (int i = 0; i < num; i++)
	{
		fscanf_s(input_para, "%s", &tempstr, 10);
		fscanf_s(input_para, "%d", &k[i]);
		fscanf_s(input_para, "%lf", &call_Bid[i]);
		fscanf_s(input_para, "%s", &tempnum);
		fscanf_s(input_para, "%lf", &call_Ask[i]);
		fscanf_s(input_para, "%s", &tempnum, 10);
		fscanf_s(input_para, "%lf", &call_Expected[i]);
		fscanf_s(input_para, "%s", &tempnum, 10);
		fscanf_s(input_para, "%s", &tempnum, 10);
		printf("C%d\t%0.2f\t%0.2f\t%0.2f\n", k[i], call_Bid[i], call_Ask[i], call_Expected[i]);

	}
	// read put option data
	for (size_t i = 0; i < num; i++)
	{
		fscanf_s(input_para, "%s", &tempstr, 10);
		fscanf_s(input_para, "%d", &k[i]);
		fscanf_s(input_para, "%lf", &put_Bid[i]);
		fscanf_s(input_para, "%s", &tempnum, 10);
		fscanf_s(input_para, "%lf", &put_Ask[i]);
		fscanf_s(input_para, "%s", &tempnum, 10);
		fscanf_s(input_para, "%lf", &put_Expected[i]);
		fscanf_s(input_para, "%s", &tempnum, 10);
		fscanf_s(input_para, "%s", &tempnum, 10);
		printf("C%d\t%0.2f\t%0.2f\t%0.2f\n", k[i], put_Bid[i], put_Ask[i], put_Expected[i]);
	}

	std::fclose(input_para);
	printf("\nS&P500:%lf\nExp.Return:%lf\nInvestment horizon:%0.0f\ntheta:%0.2f\n", S_0, S_T, horizon, theta);
	printf("Data loading is complete!\n", S_0);

	float c;
	cout << "\nInput Chance Level (0 ~ 0.5 ~ [0,1]):";
	cin >> c;

	//see.p774 table ,here is 50%, the optimal solutions results is Exp Return = 1.77%
	IloNum chance = c;

#pragma endregion



#pragma region(S) Obj Exp here:

	IloNumExpr sum_Exp(env);
	for (int i = 0; i < num; i++) {
		sum_Exp += (call_x_Ask[i] * call_Expected[i] + put_x_Ask[i] * put_Expected[i]);
		sum_Exp += (call_x_Bid[i] * call_Expected[i] + put_x_Bid[i] * put_Expected[i]);
	}

	// Obj
	model.add(IloMaximize(env, sum_Exp + y * 765.32 + z * (1 + r)));

#pragma endregion



#pragma region(S) constraints Exp here:

	IloNumExpr sum_Ask(env);
	for (int i = 0; i < num; i++) {
		sum_Ask += (call_x_Ask[i] * call_Ask[i] + put_x_Ask[i] * put_Ask[i]);
	}

	IloNumExpr sum_Bid(env);
	for (int i = 0; i < num; i++) {
		sum_Bid += (call_x_Bid[i] * call_Bid[i] + put_x_Bid[i] * put_Bid[i]);
	}

	// contraints (17)  see.p773
	IloAdd(model, sum_Ask + sum_Bid + y * S_0 + z <= cash);

	//CPLEX syntax, create an cplex exp Object
	IloNumExprArray v_fun_exp_array(env);

	//CPLEX syntax, create an expression
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < num; i++) {
		v_fun_exp_array[0] += (put_x_Ask[i] + put_x_Bid[i]) * IloMax(0, (k[i] - 0));
		v_fun_exp_array[0] += (call_x_Ask[i] + call_x_Bid[i]) * IloMax(0, (0 - k[i]));
	}
	// see.p771 'the present value of estimated divdends is $1.59'
	v_fun_exp_array[0] += (z * (1 + r) + y * (0 + 1.59));

	// constraints (18)
	IloAdd(model, v_fun_exp_array[0] >= cash * theta);


	// constraints (19)
	IloInt t_k = k[0];
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < num; i++) {
		v_fun_exp_array[1] += (put_x_Ask[i] + put_x_Bid[i]) * IloMax(0, (k[i] - t_k));
		v_fun_exp_array[1] += (call_x_Ask[i] + call_x_Bid[i]) * IloMax(0, (t_k - k[i]));
	}
	v_fun_exp_array[1] += (z * (1 + r) + y * (t_k + 1.59));
	IloAdd(model, v_fun_exp_array[1] >= v_fun_exp_array[0]);


	// constraints (20)
	for (int j = 1; j < num - 1; j++) {
		t_k = k[j];
		v_fun_exp_array.add(IloNumExpr(env));
		for (int i = 0; i < num; i++) {
			v_fun_exp_array[j + 1] += (put_x_Ask[i] + put_x_Bid[i]) * IloMax(0, (k[i] - t_k));
			v_fun_exp_array[j + 1] += (call_x_Ask[i] + call_x_Bid[i]) * IloMax(0, (t_k - k[i]));
		}
		v_fun_exp_array[j + 1] += (z * (1 + r) + y * (t_k + 1.59));
		IloAdd(model, v_fun_exp_array[j + 1] >= v_fun_exp_array[j]);
	}


	// constraints (23)
	IloNumExpr  sum_callx(env);
	for (int i = 0; i < num; i++) {
		sum_callx += call_x_Ask[i];
	}
	IloAdd(model, y + sum_callx >= 0);


	/* 
	constraints (21-22) 
		Here should be use the cumulative distribution function for valuation,
		lognormally distributed: X ~N(6.640253,0.01136)

		$f_{\lg -N}(x ; \mu, \sigma)=\frac{1}{x \sigma \sqrt{2 \pi}} e^{-\frac{(\ln x-\mu)^{2}}{2 \sigma^{2}}}$
		-----
		$E(X)=e^{\mu+\sigma^{2} / 2}$
		$D(X)=\left(e^{\sigma^{2}}-1\right) e^{2 \mu+\sigma^{2}}$

		----
		$\mu=\ln [E(X)]-\frac{1}{2} \ln \left[1+\frac{D(X)}{E(X)^{2}}\right]$
		$\sigma=\sqrt{\ln \left[1+\frac{D(X)}{E(X)^{2}}\right]}$
	*/
	IloNum u_k = 813 - 100 * chance;
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < num; i++) {
		v_fun_exp_array[num] += (put_x_Ask[i] + put_x_Bid[i]) * IloMax(0, (k[i] - u_k));
		v_fun_exp_array[num] += (call_x_Ask[i] + call_x_Bid[i]) * IloMax(0, (u_k - k[i]));
	}
	v_fun_exp_array[num] += z * (1 + r) + y * (u_k + 1.59);
	IloAdd(model, v_fun_exp_array[num] >= 100);

#pragma endregion



#pragma region(S) CPLEX solve

	// CPLEX syntax, create objective function
	IloCplex cpl(model);

	// CPLEX syntax, Solve problem
	cpl.solve();

	// CPLEX syntax, Export model
	cpl.exportModel("model.lp");

#pragma endregion



#pragma region(S) output optimal solution  

	for (size_t i = 0; i < num; i++)
	{
		printf("C-%d\t%lf\t%lf\n", k[i], cpl.getValue(call_x_Ask[i]), cpl.getValue(call_x_Bid[i]));
	}

	for (size_t i = 0; i < num; i++)
	{
		printf("P-%d\t%lf\t%lf\n", k[i], cpl.getValue(put_x_Ask[i]), cpl.getValue(put_x_Bid[i]));
	}

	printf("y: %lf\n", cpl.getValue(y));
	printf("z: %lf\n", cpl.getValue(z));
	printf("Exp.Return: %0.2f%%\n", (cpl.getObjValue() - cash) / cash * 100);

#pragma endregion

	system("pause");
	return 0;
}