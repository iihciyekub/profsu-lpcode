#include <iostream>
#include <stdlib.h>
#include<ilcplex/ilocplex.h>
ILOSTLBEGIN

int main()
{
#pragma region(S) init set
	IloEnv env;
	IloModel model(env);
	IloNumVarArray call_x_p(env), put_x_p(env);
	IloNumVarArray call_x_n(env), put_x_n(env);




	for (size_t i = 0; i < 7; i++)
	{
		call_x_p.add(IloNumVar(env, 0, IloInfinity, ILOFLOAT));
		put_x_p.add(IloNumVar(env, 0, IloInfinity, ILOFLOAT));		
		call_x_n.add(IloNumVar(env, -10, 0, ILOFLOAT));
		put_x_n.add(IloNumVar(env, -10, 0, ILOFLOAT));

	}

	IloNumVar y(env, 0, IloInfinity, ILOFLOAT), z(env, 0, IloInfinity, ILOFLOAT);


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
	IloNum Dividends = 1.59;
	IloNum horizon = 23;
	IloNum Day_of_year = 360;


#pragma region(S) input data

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

#pragma endregion(E)


#pragma region(S) Obj Exp here:

	IloNumExpr sum_Exp(env);
	for (int i = 0; i < 7; i++) {
		sum_Exp += (call_x_p[i] * call_Expected[i] + put_x_p[i] * put_Expected[i]);
		sum_Exp += (call_x_n[i] * call_Expected[i]  + put_x_n[i] * put_Expected[i]);
	}

	// Obj
	model.add(IloMaximize(env, sum_Exp + y * 765.32 + z * (1 + r)));

#pragma endregion








#pragma region(S) constraints Exp here:


	IloNumExpr sum_Ask(env);
	for (int i = 0; i < 7; i++) {
		sum_Ask += (call_x_p[i] * call_Ask[i]  + put_x_p[i] * put_Ask[i]);
	}

	IloNumExpr sum_Bid(env);
	for (int i = 0; i < 7; i++) {
		sum_Bid += (call_x_n[i] * call_Bid[i]  + put_x_n[i] * put_Bid[i]);
	}

	// contraints (17)
	IloAdd(model, sum_Ask + sum_Bid + y * S_0 + z <= cash);


	IloNumExprArray v_fun_exp_array(env);

	//创建一个表达式
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < 7; i++) {
		v_fun_exp_array[0] += (put_x_p[i] + put_x_n[i]) * IloMax(0, (k[i] - 0));
		v_fun_exp_array[0] += (call_x_p[i] + call_x_n[i]) * IloMax(0, (0 - k[i]));
	}
	v_fun_exp_array[0] += (z * (1 + r)+ y * (0+1.59));

	// constraints (18)
	IloAdd(model, v_fun_exp_array[0] >= cash * theta);





	// constraints (19)
	IloInt t_k = k[0];
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < 7; i++) {
		v_fun_exp_array[1] += (put_x_p[i] + put_x_n[i]) * IloMax(0, (k[i] - t_k));
		v_fun_exp_array[1] += (call_x_p[i] + call_x_n[i]) * IloMax(0, (t_k - k[i]));
	}
	v_fun_exp_array[1] += (z * (1 + r) + y * (t_k + 1.59));
	IloAdd(model, v_fun_exp_array[1] >= v_fun_exp_array[0]);








	// constraints (20)
	for (int j = 1; j <6; j++) {
		t_k = k[j];
		v_fun_exp_array.add(IloNumExpr(env));
		for (int i = 0; i < 7; i++) {
			v_fun_exp_array[j + 1] += (put_x_p[i] + put_x_n[i]) * IloMax(0, (k[i] - t_k));
			v_fun_exp_array[j + 1] += (call_x_p[i] + call_x_n[i]) * IloMax(0, (t_k - k[i]));
		}
		v_fun_exp_array[j+1] += (z * (1 + r) + y * (t_k + 1.59));
		IloAdd(model, v_fun_exp_array[j+1] >= v_fun_exp_array[j]);
	}



	
	// constraints (20)
	IloNumExpr  sum_callx(env);
	for (int i = 0; i < 7; i++) {
		sum_callx += call_x_p[i];
	}
	IloAdd(model, y + sum_callx >= 0);
	

	
	t_k = 760;
	v_fun_exp_array.add(IloNumExpr(env));
	for (int i = 0; i < 7; i++) {
		v_fun_exp_array[7] += (put_x_p[i] + put_x_n[i]) * IloMax(0, (k[i] - t_k));
		v_fun_exp_array[7] += (call_x_p[i] + call_x_n[i]) * IloMax(0, (t_k - k[i]));
	}
	v_fun_exp_array[7] += z * (1 + r) + y  * (t_k + 1.59);
	IloAdd(model, v_fun_exp_array[7] >= 100);

#pragma endregion
	cpl.solve();
	cpl.exportModel("model.lp");








#pragma region (S) output optimal solution  

	for (size_t i = 0; i < 7; i++)
	{
		printf("C-%d\t%lf\t%lf\n", k[i], cpl.getValue(call_x_p[i]), cpl.getValue(call_x_n[i]));
	}

	for (size_t i = 0; i < 7; i++)
	{
		printf("P-%d\t%lf\t%lf\n", k[i], cpl.getValue(put_x_p[i]), cpl.getValue(put_x_n[i]));
	}

	printf("%s %lf %s","y:", cpl.getValue(y),"\n");
	printf("%s %lf %s", "z:", cpl.getValue(z), "\n");
	printf("%s %lf %s", "Obj:", cpl.getObjValue(), "\n");

#pragma endregion

	system("pause");
}
