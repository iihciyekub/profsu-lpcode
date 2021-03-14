/*
Cees Dert, Bart Oldenkamp, (2000) Optimal Guaranteed Return Portfolios and the Casino Effect.
Operations Research 48(5) : 768 - 775. http ://dx.doi.org/10.1287/opre.48.5.768.12400

GitHub : https://github.com/iihciyekub/profsu-lpcode

Author: Li,YougJian
March 14,2021 
IDE : Microsoft Visual Studio Professional 2019 Ver 16.8.1
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
	IloModel model(env);

	// Buy CALL or PUT option (see,P769) '(positive) Amount of put and call options in the portfolio'
	IloNumVarArray call_x_Ask(env), put_x_Ask(env);
	// Sell CALL or PUT option (see,P769) '(negative) Amount of put and call options in the portfolio'
	IloNumVarArray call_x_Bid(env), put_x_Bid(env);

	// CPLEX syntax, create variables
	for (size_t i = 0; i < 7; i++)
	{
		call_x_Ask.add(IloNumVar(env, 0, IloInfinity, ILOFLOAT));
		put_x_Ask.add(IloNumVar(env, 0, IloInfinity, ILOFLOAT));
		call_x_Bid.add(IloNumVar(env, -10, 0, ILOFLOAT));
		put_x_Bid.add(IloNumVar(env, -10, 0, ILOFLOAT));

	}
	// CPLEX syntax: create  variables. (see,P769) 'number of units of the index in the portfolio'
	IloNumVar y(env, 0, IloInfinity, ILOFLOAT);
	// CPLEX syntax: create  variables. (see,P769) 'Amount of money invested in the risk-free
	IloNumVar z(env, 0, IloInfinity, ILOFLOAT);

	// CPLEX syntax, create objective function
	IloCplex cpl(model);


	int num = 7;
	IloIntArray k(env, num);
	IloNumArray call_Bid(env, num), call_Ask(env, num), call_Expected(env, num);
	IloNumArray put_Bid(env, num), put_Ask(env, num), put_Expected(env, num);

	IloNum S_0 = 760.48;
	IloNum S_T = 765.32;

	IloNum cash = 100;
	IloNum theta = 0.9;
	IloNum r = 0.0037;
	IloNum mu_by_year = 765.32;
	IloNum rate_by_year = 0.0037;
	IloNum SD_by_year = 0.179;
	// see.p771 
	IloNum Dividends = 1.59;
	IloNum horizon = 23;
	// this paper sets the total number of days of year is 360
	IloNum Day_of_year = 360;


	//see.p774 table ,here is 50%, the optimal solutions results is Exp Return = 1.77%

#pragma endregion



#pragma region(S) load data

	FILE* input_para;
	char tempstr[256];
	double tempnum;
	auto filepath = "./data.txt";
	while (fopen_s(&input_para, filepath, "r") != 0)
	{
		cout << "当前目录中没有找到 Parameters.txt..... \n放置文件后,请按回车继续" << endl;
		system("pause");
	}
	// skip the first raw
	fgets(tempstr, 255, input_para);
	// read call option data
	for (int i = 0; i < 7; i++)
	{
		fscanf_s(input_para, "%s", &tempstr, 10);
		fscanf_s(input_para, "%d", &k[i]);
		fscanf_s(input_para, "%lf", &call_Bid[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &call_Ask[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &call_Expected[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &tempnum);
		env.out() << "C" << k[i] << "\t" << call_Bid[i] << "\t" << call_Ask[i] << "\t" << call_Expected[i] << endl;
	}


	// read put option data
	for (size_t i = 0; i < 7; i++)
	{
		fscanf_s(input_para, "%s", &tempstr, 10);
		fscanf_s(input_para, "%d", &k[i]);
		fscanf_s(input_para, "%lf", &put_Bid[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &put_Ask[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &put_Expected[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &tempnum);
		env.out() << "P" << k[i] << "\t" << put_Bid[i] << "\t" << put_Ask[i] << "\t" << put_Expected[i] << endl;

	}
	//system("pause");

	std::fclose(input_para);
	env.out() << "Data loading is complete!" << endl;

	float c;
	cout << "please input chance level (0 or 0.5 or [0,1]):";
	cin >> c;
	IloNum chance = c;

#pragma endregion



#pragma region(S) Obj Exp here:

	IloNumExpr sum_Exp(env);
	for (int i = 0; i < 7; i++) {
		sum_Exp += (call_x_Ask[i] * call_Expected[i] + put_x_Ask[i] * put_Expected[i]);
		sum_Exp += (call_x_Bid[i] * call_Expected[i] + put_x_Bid[i] * put_Expected[i]);
	}

	// Obj
	model.add(IloMaximize(env, sum_Exp + y * 765.32 + z * (1 + r)));

#pragma endregion



#pragma region(S) constraints Exp here:


	IloNumExpr sum_Ask(env);
	for (int i = 0; i < 7; i++) {
		sum_Ask += (call_x_Ask[i] * call_Ask[i] + put_x_Ask[i] * put_Ask[i]);
	}

	IloNumExpr sum_Bid(env);
	for (int i = 0; i < 7; i++) {
		sum_Bid += (call_x_Bid[i] * call_Bid[i] + put_x_Bid[i] * put_Bid[i]);
	}

	// contraints (17)  see.p773
	IloAdd(model, sum_Ask + sum_Bid + y * S_0 + z <= cash);

	//CPLEX syntax, create an cplex exp Object
	IloNumExprArray v_fun_exp_array(env);
	//CPLEX syntax, create an expression
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < 7; i++) {
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
	for (int i = 0; i < 7; i++) {
		v_fun_exp_array[1] += (put_x_Ask[i] + put_x_Bid[i]) * IloMax(0, (k[i] - t_k));
		v_fun_exp_array[1] += (call_x_Ask[i] + call_x_Bid[i]) * IloMax(0, (t_k - k[i]));
	}
	v_fun_exp_array[1] += (z * (1 + r) + y * (t_k + 1.59));
	IloAdd(model, v_fun_exp_array[1] >= v_fun_exp_array[0]);


	// constraints (20)
	for (int j = 1; j < 6; j++) {
		t_k = k[j];
		v_fun_exp_array.add(IloNumExpr(env));
		for (int i = 0; i < 7; i++) {
			v_fun_exp_array[j + 1] += (put_x_Ask[i] + put_x_Bid[i]) * IloMax(0, (k[i] - t_k));
			v_fun_exp_array[j + 1] += (call_x_Ask[i] + call_x_Bid[i]) * IloMax(0, (t_k - k[i]));
		}
		v_fun_exp_array[j + 1] += (z * (1 + r) + y * (t_k + 1.59));
		IloAdd(model, v_fun_exp_array[j + 1] >= v_fun_exp_array[j]);
	}


	// constraints (23)
	IloNumExpr  sum_callx(env);
	for (int i = 0; i < 7; i++) {
		sum_callx += call_x_Ask[i];
	}
	IloAdd(model, y + sum_callx >= 0);



	// constraints (21-22)
	IloNum u_k = 813 - 100 * chance;
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < 7; i++) {
		v_fun_exp_array[7] += (put_x_Ask[i] + put_x_Bid[i]) * IloMax(0, (k[i] - u_k));
		v_fun_exp_array[7] += (call_x_Ask[i] + call_x_Bid[i]) * IloMax(0, (u_k - k[i]));
	}
	v_fun_exp_array[7] += z * (1 + r) + y * (u_k + 1.59);
	IloAdd(model, v_fun_exp_array[7] >= 100);


#pragma endregion



#pragma region(S) CPLEX solve

	cpl.solve();
	cpl.exportModel("model.lp");

#pragma endregion



#pragma region (S) output optimal solution  

	for (size_t i = 0; i < 7; i++)
	{
		printf("C-%d\t%lf\t%lf\n", k[i], cpl.getValue(call_x_Ask[i]), cpl.getValue(call_x_Bid[i]));
	}

	for (size_t i = 0; i < 7; i++)
	{
		printf("P-%d\t%lf\t%lf\n", k[i], cpl.getValue(put_x_Ask[i]), cpl.getValue(put_x_Bid[i]));
	}

	printf("y: %lf\n", cpl.getValue(y));
	printf("z: %lf\n",cpl.getValue(z));
	printf( "Exp.Return: %0.2f%%\n", (cpl.getObjValue()-cash)/cash *100);

#pragma endregion

	system("pause");
	return 0;
}
