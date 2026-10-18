USE [GunzDB]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

CREATE OR ALTER FUNCTION [dbo].[fnCheckString](@P_String VARCHAR(500))
RETURNS BIT
AS
BEGIN
    DECLARE @V_RetValue BIT
    DECLARE @V_Position INT

    SET @V_Position = 1
    SET @V_RetValue = 0   

    WHILE @V_Position <= DATALENGTH(@P_String)
          AND @V_RetValue = 0
    BEGIN
        IF (
            ASCII(SUBSTRING(@P_String, @V_Position, 1)) >= 48 AND
            ASCII(SUBSTRING(@P_String, @V_Position, 1)) <= 122 AND
            ASCII(SUBSTRING(@P_String, @V_Position, 1)) != 94
        )
            SELECT @V_RetValue = 0
        ELSE
            SELECT @V_RetValue = 1  

        SET @V_Position = @V_Position + 1
    END

    RETURN @V_RetValue
END
GO
